module main;

   wire [3:0] a, b;
   wire [3:0] out;

   subtract dut (.a(a), .b(b), .out_sig(out));

   reg [8:0]  test_vector;
   assign {a, b} = test_vector;

   initial begin
      for (test_vector=0 ; test_vector[8]==0 ; test_vector=test_vector+1) begin
	 #1 if (out != a-b) begin
	    $display("FAILED -- out=%b, expecting %b-%b=%b", out, a, b, a-b);
	    $finish;
	 end
      end

      $display("PASSED");
      $finish;
   end // initial begin
endmodule // main
