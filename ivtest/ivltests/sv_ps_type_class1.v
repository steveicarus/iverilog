// Check that class types declared in a package can be referenced using a scoped
// type identifier.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;

  localparam A = 8;

  class C;
    logic [A-1:0] x;

    task t;
      `check($bits(x), 8)
      if (!failed) begin
        $display("PASSED");
      end
    endtask
  endclass

endpackage

module test;

  localparam A = 4;
  P::C c;

  initial begin
    c = new;
    c.t();
  end

endmodule
