// Github Issue #202

module main;
   reg clk = 0;

   initial begin
      //$dumpfile("foo"); // We really want to test the command line flag
      $dumpvars(0, main);
      #10 clk <= 1;
      #10 clk <= 0;
      #10 $finish;

   end

endmodule // main
