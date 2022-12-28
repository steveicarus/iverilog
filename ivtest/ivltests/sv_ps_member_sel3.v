// Check that indices to a property for package scoped identifier get evaluated
// in the scope where the identifier is used, not where the identifier is
// declared.

package P;
  localparam N = 1;
  class C;
    localparam X = 4'b0101;
  endclass
  C c = new;
endpackage

module test;
  localparam N = 2;

  initial begin
    if (P::c.X[N] === 1'b1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
