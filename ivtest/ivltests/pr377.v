module dummy (.B(A[2:1]));
  input [2:1] A;
always @(A)
   $display (A);
endmodule

module test ();
reg [2:0] A;

dummy dummy(A[1:0]);

integer idx;
initial begin
   for (idx = 0 ;  idx <= 'h7 ;  idx = idx+1)
      #1 A <= idx;

   #1 $finish(0);
end
endmodule
