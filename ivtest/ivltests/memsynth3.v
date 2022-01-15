`begin_keywords "1364-2005"
module main;

   reg [3:0] foo, bar;
   reg [1:0] adr;

   reg	     bit, rst, clk;
   reg	     load_enable, write_enable;

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge rst)
     if (rst) begin
	foo <= 0;

     end else if (load_enable) begin
	foo <= bar;

     end else if (write_enable) begin
	foo[adr] <= bit;

     end

   (* ivl_synthesis_off *)
   initial begin
      rst = 1;
      clk = 0;
      bar = 4'bzzzz;
      load_enable = 0;
      write_enable = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (foo !== 4'b0000) begin
	 $display("FAILED -- reset foo=%b", foo);
	 $finish;
      end

      rst = 0;
      bar = 4'b1001;
      load_enable = 1;
      write_enable = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (foo !== bar) begin
	 $display("FAILED -- load foo=%b, bar=%b", foo, bar);
	 $finish;
      end

      load_enable = 0;
      write_enable = 0;

      #1 clk = 1;
      #1 clk = 0;
      if (foo !== 4'b1001) begin
	 $display("FAILED -- foo=%b after clk", foo);
	 $finish;
      end

      adr = 1;
      bit = 1;
      load_enable = 0;
      write_enable = 1;

      #1 clk = 1;
      #1 clk = 0;

      if (foo !== 4'b1011) begin
	 $display("FAILED -- foo=%b, adr=%b, bit=%b", foo, adr, bit);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
`end_keywords
