
module main;

   typedef struct packed {
      logic [3:0] adr;
      logic [3:0] val;
   } foo_s;

   foo_s [3:0][1:0] ival;
   foo_s [3:0][1:0] oval;

   genvar	    g;
   for (g = 0 ; g < 4 ; g = g+1) begin:loop
      TEST dut(.in(ival[g][0]),
	       .out(oval[g][0]));
   end

   initial begin
      ival = 'hx3x2x1x0;
      #1 $display("ival = %h, oval = %h", ival, oval);
      if (oval !== 64'hzzxxzzxxzzxdzzxf) begin
	 $display("FAILED -- oval=%h", oval);
	 $finish;
      end
      $display("PASSED");
      $finish;
   end

endmodule // main

module TEST (input wire [7:0] in, output wire [7:0] out);
   assign out = ~in;
endmodule // TEST
