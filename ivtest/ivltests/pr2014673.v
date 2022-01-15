// I ran this with "iverilog -y. bugreport.v -s bugreport && ./a.out"
// on Icarus Verilog version 0.9.devel (s20080429)
// and got "Bug observed: Got xxxxx, expected 0004b."
// (some code taken from async_transmitter.v at http://www.fpga4fun.com)
module bugreport;

   parameter ClkFrequency = 50000000;
   parameter Baud = 57600;
   parameter BaudGeneratorAccWidth = 16;
   wire [BaudGeneratorAccWidth:0] BaudGeneratorInc = ((Baud<<(BaudGeneratorAccWidth-4))+(ClkFrequency>>5))/(ClkFrequency>>4);
   wire [BaudGeneratorAccWidth:0] BaudGeneratorIncShouldBe = 17'h4b;

   initial #1 if (BaudGeneratorInc !== BaudGeneratorIncShouldBe)
     $display("FAILED -- Got %x, expected %x.",BaudGeneratorInc,BaudGeneratorIncShouldBe);
   else
     $display("PASSED");

endmodule // bugreport
