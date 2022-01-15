
module test_logic
  #(parameter WID = 4)
   (input wire [WID-1:0] A,
    output q_and, q_or, q_xor, q_nand, q_nor, q_xnor);

   assign q_and =  & A;
   assign q_or  =  | A;
   assign q_xor =  ^ A;
   assign q_nand= ~& A;
   assign q_nor = ~| A;
   assign q_xnor= ~^ A;

endmodule // test_logic
