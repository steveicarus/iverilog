/*
 * Author: Oswaldo Cadenas <oswaldo.cadenas@gmail.com>
 *
 * The test checks the module bit ouput type accepts default
 * initialization value.
 */

module clkgen(output bit clk = 0);

initial begin
   #100;
   disable checking;
   disable gen;
   $display ("PASSED");
   $finish;
end

initial begin
  fork
    gen;
    checking;
  join
end

task gen;
   forever #10 clk = ~clk;
endtask

task checking;
   forever begin
      #1;
      if (clk ==! 1'b0 && clk ==! 1'b1 ) begin
         $display ("FAILED!");
	     $finish;
      end
   end
endtask

endmodule
