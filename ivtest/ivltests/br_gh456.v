

module testr(input clk, output real out, input wire real in);
   always @(posedge clk)
     out <= in + 1.0;
endmodule // testr

module testi(input clk, output int out, input wire int in);
   always @(posedge clk)
     out <= in + 1;
endmodule // testi

module main;

   reg  clk;
   always begin
      #10 clk = 0;
      #10 clk = 1;
   end

   real srcr;
   int	srci;

   wire real valr;
   wire int  vali;
   testr r0 (.clk(clk), .out(valr), .in(srcr));
   testi i0 (.clk(clk), .out(vali), .in(srci));

   real refr;
   int  refi;
   always @(posedge clk) begin
      refr <= srcr + 1.0;
      refi <= srci + 1;
   end

   initial begin
      @(negedge clk) ;
      srcr = 2.0;
      srci = 2;
      @(negedge clk) ;
      @(negedge clk) ;

      $display("srcr=%f, valr=%f, refr=%f", srcr, valr, refr);
      if (valr != refr && valr != 3.0) begin
	 $display("FAILED");
	 $finish;
      end

      $display("srci=%0d, vali=%0d, refi=%0d", srci, vali, refi);
      if (vali != refi && vali != 3) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
