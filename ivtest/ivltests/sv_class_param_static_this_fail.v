// A static method has no object. Check that using this, and calling a method
// that is not static without a scope, are reported in a static method.

class C;
  int x;

  function int get();
    return x;
  endfunction

  function void put();
  endfunction

  task run();
  endtask

  static function int b1();
    return this.x;
  endfunction

  static function int b2();
    this.x = 1;
    return 0;
  endfunction

  static function int b3();
    return this == null;
  endfunction

  static function int b4();
    return get();
  endfunction

  static function int b5();
    put();
    return 0;
  endfunction

  static task b6();
    run();
  endtask

  static function int b7();
    begin : blk
      int y;
      y = get();
      return y;
    end
  endfunction
endclass

module test;
  initial $display("FAILED");
endmodule
