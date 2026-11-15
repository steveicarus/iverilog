// Calling a static class method through the class scope is not supported
// yet. Check that it is reported, both as a function call and as a task call.

class C #(int N = 4);
  static function int get();
    return N;
  endfunction
endclass

module test;

  int x;

  initial begin
    x = C#(8)::get();
    C#(8)::get();
    $display("FAILED");
  end

endmodule
