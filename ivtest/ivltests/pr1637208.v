/* PR1637208 */

module main;
   reg clock;
   reg [31:0] pixel0;
   reg [31:0] mem [0:1];

   always @(posedge clock) begin
      mem[0] <= pixel0;
   end

   always @(posedge clock) begin
      mem[1] <= mem[0];
   end

   reg sel;
   wire [31:0] foo = sel? mem[1] : mem[0];

   initial begin
      clock = 1;
      sel = 0;
      #1 pixel0 = 'h55555555;
      #1 clock = 0;
      #1 clock = 1;
      #1 pixel0 = 'haaaaaaaa;
      #1 clock = 0;
      #1 clock = 1;
      #1 if (mem[0] !== 32'haaaaaaaa) begin
	 $display("FAILED -- mem[0] = %h", mem[0]);
	 $finish;
      end

      if (mem[1] !== 32'h55555555) begin
	 $display("FAILED == mem[1] = %h", mem[1]);
	 $finish;
      end

      if (foo !== mem[0]) begin
	 $display("FAILED -- mem[sel=0] != %h", foo);
	 $finish;
      end

      sel = 1;

      #1 if (foo !== mem[1]) begin
	 $display("FAILED -- mem[sel=1] != %h", foo);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
