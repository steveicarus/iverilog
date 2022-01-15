module test(CLK, OE, A, OUT);
parameter numAddr	= 1;
parameter numOut	= 1;
parameter wordDepth	= 2;
parameter MemFile       = "ivltests/pr690.dat";

input CLK, OE;
input [numAddr-1:0] A;
output [numOut-1:0] OUT;

reg [numOut-1:0] memory[wordDepth-1:0];
reg [numAddr-1:0] addr;

initial begin
   // The whole point of this regression test is to check that
   // the file name argument can be a string parameter.
   $readmemb(MemFile,memory, 0);
   if (memory[0] !== 0) begin
      $display("FAILED -- memory[0] == %b", memory[0]);
      $finish;
   end
   if (memory[1] !== 1) begin
      $display("FAILED -- memory[1] == %b", memory[1]);
      $finish;
   end

   $display("PASSED");
end


endmodule
