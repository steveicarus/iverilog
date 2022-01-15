module top;

   wire net1, net2;
   reg [1:0] data;

   buf bus_drv[1:0] (net1, net2, data);

   initial begin
      data = 0;
      #1 $monitor(net1,,net2,,data);
      #1 if (net1 !== 1'b0) begin
	 $display("FAILED");
	 $finish;
      end

      data = 3;
      #1 if (net1 !== 1'b1) begin
	 $display("FAILED");
	 $finish;
      end

      data = 1;
      #1 if (net1 !== 1'bx) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
