// Check that a wildcard-imported parameter works in a port connection.

package p;
  localparam [1:0] L = 2'b10;
endpackage

module M(input wire [1:0] value, output wire [1:0] result);
  assign result = value;
endmodule

module test;
  import p::*;

  wire [1:0] result;
  M i_m(.value(L), .result(result));

  initial begin
    #1;
    if (result !== 2'b10) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end
endmodule
