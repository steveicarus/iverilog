// Check that indices to a package scoped identifier get evaluated in the scope
// where the identifier is used, not where the identifier is declared.

package P;
  localparam N = 1;
  logic [3:0] x = 4'b0101;
endpackage

module test;
  localparam N = 2;
  initial begin
    if (P::x[N] === 1'b1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
