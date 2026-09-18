// Check that a const static property cannot be assigned by local lookup.

module test;

  class C;
    const static int value = 42;

    static function void write_value;
      value = 1;
    endfunction
  endclass

endmodule
