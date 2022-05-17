
//
// This tests that the parameter and localparam show up in the vcd dump.
//
module main;
   parameter [3:0] foo = 4'd5;
   localparam [3:0] bar = 7;
   wire [3:0] bat = foo + bar;

   initial begin
      $dumpfile("work/br_gh156.vcd");
      $dumpvars(0, main);
      #1 $finish;
      
   end
   
endmodule // main
