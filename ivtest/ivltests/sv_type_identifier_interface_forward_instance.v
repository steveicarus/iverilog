// Check that a forward interface and a same-named type can both be used.

package p;
  typedef logic [7:0] I;
endpackage

module test;
  import p::*;

  I i_i(); // Instantiates interface.
  I value; // Declares variable.

  initial begin
    value = 8'h2a;
    if (value !== 8'h2a) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule

interface I;
endinterface
