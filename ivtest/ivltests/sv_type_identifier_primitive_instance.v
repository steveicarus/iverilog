// Check that a UDP and a same-named type can both be used.

package p;
  typedef logic [7:0] P;
endpackage

primitive P(out, in);
  output out;
  input in;
  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

module test;
  import p::*;

  reg in;
  wire out;

  P i_p(out, in); // Instantiates primitive.
  P value; // Declares variable.

  initial begin
    value = 8'h2a;
    in = 1'b1;
    #1;
    if (out !== 1'b1 || value !== 8'h2a) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end
endmodule
