tmp="$(mktemp)"
file="test/cli/autocorrect-bare-stdlib-generics/autocorrect-bare-stdlib-generics.rb"
cp "$file" "$tmp"
main/sorbet --silence-dev-message -a "$tmp"
diff "$file" "$tmp"
rm "$tmp"