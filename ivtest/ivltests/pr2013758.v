// pr2013758

module test;

   reg reset;
   initial begin
//      $dumpfile( "test.vcd" );
//      $dumpvars;

      reset = 0;
      #100;
      reset = 1;
      #100;
      reset = 0;
      #100 $display("PASSED");
      $finish;
   end

   submod1 s1 (.reset(reset));
   submod2 s2 (.reset(reset));

endmodule

module submod1(input reset);
   wire reset2 = 1;
   assign reset2 = reset;
endmodule

module submod2(input reset);

   always #10 @(reset)
     if (reset === 1'bx) begin
	$display("FAILED -- X escaped into sibling module!");
	$finish;
     end
endmodule
