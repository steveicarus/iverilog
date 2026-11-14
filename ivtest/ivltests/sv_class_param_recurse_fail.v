// A class that specializes itself with a changing value would make
// specializations forever. Check that it is reported once.

class C #(int N = 1);
  function void f();
    C#(N+1) x;
    x = new;
    x.f();
  endfunction
endclass

module test;

  initial begin
    C#(1) c;
    c = new;
    c.f();
    $display("FAILED");
  end

endmodule
