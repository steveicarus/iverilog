module main;

   wire [7:0] bus;
   reg [7:0]  HiZ;
   assign     bus = HiZ;

   reg	      E;
   reg	      D;
   reg	      CLK;
   BUFT drv (bus[0], D, E, CLK);

   bufif0 drv0 (bus[0], D, E);

   initial begin
      HiZ = 8'hzz;
      D = 1;
      E = 1;
      CLK = 0;
      #1 CLK = 1;
      #1 if (bus !== 8'bzzzzzzz1) begin
	 $display("FAILED");
	 $finish;
      end

      if (drv.D !== D) begin
	 $display("FAILED (D)");
	 $finish;
      end

      E = 0;
      #1 if (bus !== 8'bzzzzzzz1) begin
	 $display("FAILED");
	 $finish;
      end

      D = 0;
      CLK = 0;
      #1 CLK = 1;

      if (drv.D !== D) begin
	 $display("FAILED (D)");
	 $finish;
      end

      #1 D = 1;
      #1 if (bus !== 8'bzzzzzzz1) begin
	 $display("FAILED");
	 $finish;
      end

      if (drv.D !== D) begin
	 $display("FAILED (D)");
	 $finish;
      end
      $display("bus=%b, D=%b, drv.D=%b, E=%b, drv.save=%b", bus, D, drv.D, E, drv.save);
      $display("PASSED");
   end // initial begin

endmodule // main

module BUFT(inout wire TO, input wire D, input wire E, input wire CLK);

   reg save;
   assign TO = E? save : 2'bz;

   always @(posedge CLK)
     save <= D;

endmodule // BUFT
