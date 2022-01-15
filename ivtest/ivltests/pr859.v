/*
 * Based on pr#859.
 * This test makes sure the @* input search can handle null
 * statements within case alternatives.
 */

module test(output reg [15:0] probe_data,input wire [3:0] probe_sel);

   always @*
     case(probe_sel)
       4'h0 :;
       default : probe_data = 16'b0;
     endcase // case(probe_sel)

   initial #1 $display("PASSED");

endmodule // test
