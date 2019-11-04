# typed: true

def foo
  # ^^^ symbol-search: "foo"
end

class Foo
  #   ^^^ symbol-search: "Foo"
  #   ^^^ symbol-search: "foo"
  def foo
    # ^^^ symbol-search: "foo"
  end
end

module Bar
  def foo
    # ^^^ symbol-search: "foo"
  end

  module Foo
    #    ^^^ symbol-search: "Foo"
    #    ^^^ symbol-search: "foo"
    def foo
      # ^^^ symbol-search: "foo"
    end
  end
end
