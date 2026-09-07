// Check that a forward module and a same-named type can both be used.

package p;
  typedef logic [7:0] M;
endpackage

module test;
  import p::*;

  wire out;

  M #(.VALUE(1'b1)) i_m(out); // Instantiates module.
  M value; // Declares variable.

  initial begin
    value = 8'h2a;
    #1;
    if (out !== 1'b1 || value !== 8'h2a) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end
endmodule

module M #(parameter VALUE = 1'b0) (output out);
  assign out = VALUE;
endmodule
