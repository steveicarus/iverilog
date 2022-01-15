module test ();

   reg [2:0] in;
   wire      Oand, Oor;

   dut #(.is_and(1)) dand (.O(Oand), .A(in[1]), .B(in[0]));
   dut #(.is_and(0)) dor  (.O(Oor ), .A(in[1]), .B(in[0]));

   initial begin
      for (in = 0 ;  in < 4 ;  in = in+1) begin
	 #1 /* settle time. */ ;
	 if (Oand !== &in[1:0]) begin
	    $display("FAILED -- in=%b, Oand=%b", in, Oand);
	    $finish;
	 end
	 if (Oor !== |in[1:0]) begin
	    $display("FAILED -- in=%b, Oor=%b", in, Oor);
	    $finish;
	 end
      end // for (in = 0 ;  in < 4 ;  in = in+1)
      $display("PASSED");
   end

endmodule

module dut (output O, input A, input B);

   parameter is_and = 1;

   generate
      if (is_and)
	and g(O, A, B);
      else
	or g(O, A, B);
   endgenerate

endmodule // dut
