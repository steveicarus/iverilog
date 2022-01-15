module main;

   genvar i;
   parameter MSB = 7;

   wire [MSB:0] Z;
   reg [MSB:0]	A, B;
   generate
      for (i = 0; i <= MSB; i = i+1)
	begin: or2
	   OR2 uor2 (.A(A[i]), .B(B[i]), .Z(Z[i]));
	end
   endgenerate

   initial begin
      for (A = 0 ;  A < 'hff ;  A = A+1)
	for (B = 0 ;  B < 'hff ;  B = B+1)
	   #1 if (Z !== (A|B)) begin
	      $display("FAILED -- A=%h, B=%h, Z=%h", A, B, Z);
	      $finish;
	   end

      $display("PASSED");
   end

endmodule

module OR2 (Z, A, B);
   output Z;
   input A;
   input B;

   or (Z, A, B);
endmodule
