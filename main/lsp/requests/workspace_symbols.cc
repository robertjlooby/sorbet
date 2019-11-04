#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/lsp.h"
#include <algorithm>
#include <regex>

using namespace std;

namespace sorbet::realmain::lsp {

const int MAX_RESULTS = 50;
/**
 * Converts a symbol into a SymbolInformation object.
 * Returns `nullptr` if symbol kind is not supported by LSP
 */
SymbolInformation *symbolRef2SymbolInformation(const LSPConfiguration &config, const core::GlobalState &gs,
                                               core::SymbolRef symRef) {
    auto sym = symRef.data(gs);
    if (!sym->loc().file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }

    auto location = config.loc2Location(gs, sym->loc());
    if (location == nullptr) {
        return nullptr;
    }
    auto result = new SymbolInformation(sym->name.show(gs), symbolRef2SymbolKind(gs, symRef), std::move(location));
    result->containerName = sym->owner.data(gs)->showFullName(gs);
    return result;
}

std::vector<unique_ptr<SymbolInformation>> getWorkspaceSymbolsForQuery(const LSPConfiguration &config,
                                                                       const core::GlobalState &gs, std::string query) {
    int limit = MAX_RESULTS;
    std::string rawRegex = "";
    int numCapturingGroups = 0;
    for (auto ch : query) {
        if (std::islower(ch)) {
            // Lowercase letters match either uppercase or lowercase
            rawRegex += fmt::format("[{}{}]", (char)toupper(ch), ch);
        } else if (std::isalnum(ch)) {
            // Uppercase characters, digits must match exactly
            rawRegex += std::string(&ch, 1);
        } else {
            // escape as a precaution against regex special characters like {}[],+*?
            rawRegex += fmt::format("\\{}", ch);
        }
        rawRegex += ".*?"; // non-greedy
        numCapturingGroups += 2;
    }
    regex weightedRegex(rawRegex);
    smatch sm;
    vector<pair<int, u4>> weightedResults;
    for (u4 idx = 1; idx < gs.symbolsUsed(); idx++) {
        core::SymbolRef symbolRef(gs, idx);
        const string shortName(symbolRef.data(gs)->name.data(gs)->shortName(gs));
        if (!regex_search(shortName, sm, weightedRegex)) {
            continue;
        }
        int distance = sm.length(); // Roughly, reward expressions where chars are more tightly packed
        weightedResults.emplace_back(distance, idx);
    }
    fast_sort(weightedResults,
              [](pair<int, u4> &left, pair<int, u4> &right) -> bool { return left.first < right.first; });
    vector<unique_ptr<SymbolInformation>> results;
    for (auto [weight, idx] : weightedResults) {
        core::SymbolRef ref(gs, idx);
        auto data = symbolRef2SymbolInformation(config, gs, ref);
        if (data != nullptr) {
            results.emplace_back(data);
            if (results.size() >= limit) {
                break;
            }
        }
    }
    return results;
}

unique_ptr<ResponseMessage> LSPLoop::handleWorkspaceSymbols(LSPTypechecker &typechecker, const MessageId &id,
                                                            const WorkspaceSymbolParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceSymbol);
    if (!config->opts.lspWorkspaceSymbolsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Workspace Symbols` LSP feature is experimental and disabled by default.");
        return response;
    }

    prodCategoryCounterInc("lsp.messages.processed", "workspace.symbols");

    string searchString(params.query);
    ShowOperation op(*config, "WorkspaceSymbols", fmt::format("Searching for symbol `{}`...", searchString));

    response->result = getWorkspaceSymbolsForQuery(*config, typechecker.state(), searchString);
    return response;
}
} // namespace sorbet::realmain::lsp
