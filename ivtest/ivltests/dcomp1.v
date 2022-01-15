/* dcomp1.v - this is a fragment of a larger program, which would
 * dynamicly compute a more interesting value for the phdelay variable.
 *
 * It illustrates a problem in verilog-20010721 when computing
 * time values for use in behavioral delays.
 */
`timescale 1ps / 1ps
module dcomp;
   time      phdelay;
   parameter clk_period = 400;
   parameter phoffset = 4;
   time      compdelay;
   reg       internal_Clk, Clk;

   initial begin
      $monitor("%b %b %t %t %t", internal_Clk, Clk, phdelay, compdelay, $time);
      phdelay = 0;
       #2000;
       phdelay = 13;
       #2001;
       $finish(0);
   end // initial begin

   initial internal_Clk <= 0;
   always #(clk_period/2) internal_Clk = ~internal_Clk;

   always @(internal_Clk) begin
// uncoment only one of the next four lines:
//      #(phdelay);             // works
//      #(phdelay + phoffset);  // fails
      compdelay = phdelay + phoffset;  #(compdelay);  // fails
//      compdelay = phdelay + 4; #(compdelay);  // fails

      $display("got here");

      Clk <= internal_Clk;

      // of course, this is what I really want... (but that's PR#105)
      //     Clk <= #(phdelay + phoffset + clk_period/2) internal_Clk;
   end // always @ (internal_Clk)

endmodule // dcomp
