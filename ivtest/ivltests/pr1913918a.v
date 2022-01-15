// pr1913918
module test();
   parameter a  = 4'b1000;
   b b(a[3]);
endmodule // test

module b(c);
   input c;
   initial #1 begin
      if (c !== 1'b1) begin
	 $display("FAILED -- c = %b", c);
	 $finish;
      end
      $display("PASSED");
   end
endmodule
