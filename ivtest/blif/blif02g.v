
module test_logic
  #(parameter WID = 4)
   (input wire [WID-1:0] A,
    output reg q_and, q_or, q_xor, q_nand, q_nor, q_xnor);

   always @(A) begin
      q_and = &A;
      q_or  = |A;
      q_xor = ^A;
      q_nand = ~q_and;
      q_nor  = ~q_or;
      q_xnor = ~q_xor;
   end

endmodule // test_logic
