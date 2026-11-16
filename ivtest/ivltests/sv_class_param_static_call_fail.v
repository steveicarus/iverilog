// Check that the errors for members that need an object are reported when
// they are used through a class scope: a method that is not static, and a
// property that is not static, also from a static method, and when they are
// inherited.

class C #(int N = 4);
  int prop;
  static int count;

  function int get();
    return N;
  endfunction

  task set(int v);
    prop = v;
  endtask

  static function int bad();
    return prop;
  endfunction
endclass

class D extends C#(2);
endclass

module test;

  int x;

  initial begin
    x = C#(8)::get();
    C#(8)::get();
    C#(8)::set(1);
    x = C::prop;
    C#(8)::prop = 1;
    x = D::get();
    C::count = 1;
    $display("FAILED");
  end

endmodule
