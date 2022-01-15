`define PERIOD 10

module main;
   reg	   CLK;
   integer counter;

   initial begin		// reset and clock generator
      counter = 0;
      CLK = 0;
      #2;			// wait 2, and then...
      repeat(10)		// generate 5 clock cycles
	#(`PERIOD/2) CLK = !CLK;
      $display("time %0t; the counter is %0d", $time, counter);
      $finish(0);
   end

   task test1;
      begin
	 @(posedge CLK);
	 $display("test1 increment; reading counter as %0d", counter);
	 // the function call is necessary to get the problem
	 counter = _$Fadd32(counter, 1'b1);
      end
   endtask

   task test2;
      begin
	 @(posedge CLK);
	 $display("test2 increment; reading counter as %0d", counter);
	 counter = _$Fadd32(counter, 1'b1);
      end
   endtask

   function [31:0] _$Fadd32;
      input l,r;
      reg [31:0] l,r;
      _$Fadd32 = l+r;
   endfunction
endmodule			// main

module trig1;
   always begin
      #`PERIOD;
      top.main.test1;
   end
endmodule

module trig2;
   always begin
      #`PERIOD;
      top.main.test2;
   end
endmodule

module top;
   main main();
   trig1 trig1();
   trig2 trig2();
endmodule
