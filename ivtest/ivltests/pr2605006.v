module test();
   reg clk;

   reg [15:0] usb_shadow [0: 32];

   initial begin
      usb_shadow[6'b0_00000] = 'b0101;
      usb_shadow[6'b1_00000] = 'b1001;
      clk = 0;
      if (usb_shadow[{!clk,5'b0}][15:2] !== 2) begin
	 $display("FAILED");
	 $finish;
      end
      clk = 1;
      if (usb_shadow[{!clk,5'b0}][15:2] !== 1) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end
endmodule // test
