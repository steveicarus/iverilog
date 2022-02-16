// Check that it is possible to declare parameters and localparams inside a
// class. Both declared parameters that can not be overridden.

module test;

class C;
  // `parameter` is also declaring a local parameter inside a class
  parameter A = 1;
  localparam B = 2;

  function bit test();
      return A == 1 && B == 2;
  endfunction

endclass

initial begin
  C c;
  c = new;

  if (c.test() && c.A == 1 && c.B == 2) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end

endmodule
