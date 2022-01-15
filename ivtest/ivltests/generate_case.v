module main;

   wire [2:0] value1, value2, value3, value4;

   dut #( .select(1) ) dut1(value1);
   dut #( .select(2) ) dut2(value2);
   dut #( .select(3) ) dut3(value3);
   dut #( .select(4) ) dut4(value4);

   initial begin
      #1 $display("value1=%d, value2=%d, value3=%d, value4=%d",
		  value1, value2, value3, value4);

      if (value1 !== 1) begin
	 $display("FAILED -- value1=%b", value1);
	 $finish;
      end

      if (value2 !== 2) begin
	 $display("FAILED -- value2=%b", value2);
	 $finish;
      end

      if (value3 !== 3) begin
	 $display("FAILED -- value3=%b", value3);
	 $finish;
      end

      if (value4 !== 7) begin
	 $display("FAILED -- value4=%b", value4);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

module dut(output wire [2:0] value);

   parameter select = 0;

   case (select)
     0: assign value = 0;
     1: assign value = 1;
     2: assign value = 2;
     3: assign value = 3;
     default:
       assign value = 7;
   endcase // case endcase

endmodule // dut
