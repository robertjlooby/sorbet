# typed: true

def foo_bar_baz
  # ^^^^^^^^^^^ symbol-search: "fbb"
end

class FooBarBaz
  #   ^^^^^^^^^ symbol-search: "FBB"
  #   ^^^^^^^^^ symbol-search: "fbb"
  # TODO: rank=1
  def foo_bar_baz
    # ^^^^^^^^^^^ symbol-search: "fbb"
    # TODO: rank=1
  end
  def other_foo_bar_baz
    # ^^^^^^^^^^^^^^^^^ symbol-search: "fbb"
    # TODO: rank=2
  end
  def xxfxxbxxbxx
    # ^^^^^^^^^^^^^^^^^ symbol-search: "fbb"
    # TODO: rank=3
  end
end

module Outer
  class FoobieArbaz
    #  ^^^^^^^^^^^ symbol-search: "fbb"
    # TODO: rank=3
    def foo_bar_baz
      # ^^^ symbol-search: "fbb", rank=1
    end
  end
end
