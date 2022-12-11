// Check that it is possible to use a package scope type identifier for the type
// of a property of class defined in the root scope

package P;
  typedef integer T;
endpackage

class C;
  P::T x;
endclass

module test;
  C c;
  initial begin
    c = new;
    c.x = 32'h55aa55aa;
    if (c.x === 32'h55aa55aa) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
