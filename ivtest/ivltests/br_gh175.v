
module dut (output reg[31:0] size,
	    output reg signed [31:0] ival,
	    output reg [31:0] hval);
   parameter string foo = "1234";
   string	    tmp;
   real		    rval;

   initial begin
      size = foo.len();
      ival = foo.atoi();
      hval = foo.atohex();
      rval = foo.atoreal();

      tmp = foo;
      $display("foo=%0s, tmp=%0s", foo, tmp);
      if (tmp != foo) begin
	 $display("FAILED");
	 $finish;
      end
      $display("rval=%f", rval);
      if (rval != ival) begin
	 $display("FAILED -- rval=%f, ival=%0d", rval, ival);
	 $finish;
      end
   end
endmodule // dut

module main;

   wire [31:0]          dut0_size, dut1_size, dut2_size;
   wire signed [31:0]   dut0_ival, dut1_ival, dut2_ival;
   wire unsigned [31:0] dut0_hval, dut1_hval, dut2_hval;


   // Instantate module with string parameter, use default value.
   dut dut0 (dut0_size, dut0_ival, dut0_hval);

   // Instantate module with string parameter, use override value.
   dut #(.foo("12345")) dut1 (dut1_size, dut1_ival, dut1_hval);

   // Instantate module with string parameter, use defparam value.
   defparam dut2.foo = "123456";
   dut dut2 (dut2_size, dut2_ival, dut2_hval);

   initial begin
      #100 ;
      $display("dut0_size=%0d", dut0_size);
      if (dut0_size !== 4) begin
	 $display("FAILED");
	 $finish;
      end
      $display("dut1_size=%0d", dut1_size);
      if (dut1_size !== 5) begin
	 $display("FAILED");
	 $finish;
      end
      $display("dut2_size=%0d", dut2_size);
      if (dut2_size !== 6) begin
	 $display("FAILED");
	 $finish;
      end
      $display("dut0_ival=%0d", dut0_ival);
      if (dut0_ival !== 1234) begin
	 $display("FAILED");
	 $finish;
      end
      $display("dut1_ival=%0d", dut1_ival);
      if (dut1_ival !== 12345) begin
	 $display("FAILED");
	 $finish;
      end
       $display("dut2_ival=%0d", dut2_ival);
      if (dut2_ival !== 123456) begin
	 $display("FAILED");
	 $finish;
      end
      $display("dut0_hval=%0h", dut0_hval);
      if (dut0_hval !== 32'h1234) begin
	 $display("FAILED");
	 $finish;
      end
      $display("dut1_hval=%0h", dut1_hval);
      if (dut1_hval !== 32'h12345) begin
	 $display("FAILED");
	 $finish;
      end
       $display("dut2_hval=%0h", dut2_hval);
      if (dut2_hval !== 32'h123456) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
      $finish;
   end
endmodule // main
