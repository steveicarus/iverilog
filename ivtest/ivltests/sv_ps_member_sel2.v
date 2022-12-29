// Check that indices to a struct member for package scoped identifier get
// evaluated in the scope where the identifier is used, not where the identifier
// is declared.

package P;
  localparam N = 1;
  struct packed {
      logic [3:0] x;
  } s = 4'b0101;
endpackage

module test;
  localparam N = 2;
  initial begin
    if (P::s.x[N] === 1'b1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
