/*
 * Based on PR#585.
 */
module main();

  reg [7:0] ram_temp;
  reg mem;

   initial begin
      ram_temp = 8'h08;
      mem = (ram_temp & 8'h08) >> 3;
      $write("Calculated: %b\nActually in mem: %b\n",((ram_temp & 8'h08) >> 3),
mem);
      if (mem !== 1'b1) begin
	 $display("FAILED == mem = %b", mem);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule
