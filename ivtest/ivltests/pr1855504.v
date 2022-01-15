// pr1855504

module mul_test();

   reg [15:0] prod;
   reg [7:0]  op2;
   reg [15:0] op1;


   initial begin
      op1 = 16'h0DA0;
      op2 = 8'h0A;
      prod = 16'h0000;
   end

   always begin
      prod <= op1[7:0] * op2;
      #5 $display("op1 = %h, op2 = %h, prod = %h", op1, op2, prod);
      #5 $finish(0);
   end

endmodule // mul_test
