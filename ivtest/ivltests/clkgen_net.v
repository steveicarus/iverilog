/*
 * Author: Oswaldo Cadenas <oswaldo.cadenas@gmail.com>
 *
 * The test checks that an unspecified output type is elaborated as Net.
 * If an intial value is given to an unspecified ouput type it does
 * not compile.
 */

module clkgen(output clk);

logic iclk = 'x;
assign clk = iclk;

initial begin
   #100;
   disable checking;
   disable gen;
   $display ("PASSED");
   $finish;
end

initial begin
    fork
      checking;
	  gen;
    join
end


task gen;
  begin
     iclk = 0;
     forever #10 iclk = ~iclk;
  end
endtask

task checking;
   forever begin
      #1;
      if (clk === 1'bx ) begin
         $display ("FAILED!");
	     $finish;
      end
   end
endtask
endmodule
