// SystemVerilog parameter type test module
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test ();

   // logic vector
   logic [15:0] lv;

   // error counter
   bit err = 0;

   // system clock (does not have any real function in the test)
   logic clk = 1;

   always #5 clk = ~clk;

   // counters
   int cnt;
   int cnt_bit ;
   int cnt_byte;
   int cnt_int ;
   int cnt_ar1d;
   int cnt_ar2d;

   // sizes
   int siz_bit ;
   int siz_byte;
   int siz_int ;
   int siz_ar1d;
   int siz_ar2d;

   // add all counters
   assign cnt = cnt_bit + cnt_byte + cnt_int + cnt_ar1d + cnt_ar2d;

   // finish report
   initial begin
      // some unnecessary delay
      wait (cnt);

      // check if variable sizes are correct
      if (siz_bit  !=  1)  begin $display("FAILED -- siz_bit  = %0d", siz_bit ); err=1; end
      if (siz_byte !=  8)  begin $display("FAILED -- siz_byte = %0d", siz_byte); err=1; end
      if (siz_int  != 32)  begin $display("FAILED -- siz_int  = %0d", siz_int ); err=1; end
      if (siz_ar1d != 24)  begin $display("FAILED -- siz_ar1d = %0d", siz_ar1d); err=1; end
      if (siz_ar2d != 16)  begin $display("FAILED -- siz_ar2d = %0d", siz_ar2d); err=1; end

      if (!err) $display("PASSED");
      $finish();
   end

   // instances with various types
   mod_typ #(.TYP (bit           )) mod_bit  (clk, cnt_bit [ 1-1:0], siz_bit );
   mod_typ #(.TYP (byte          )) mod_byte (clk, cnt_byte[ 8-1:0], siz_byte);
   mod_typ #(.TYP (int           )) mod_int  (clk, cnt_int [32-1:0], siz_int );
   mod_typ #(.TYP (bit [23:0]    )) mod_ar1d (clk, cnt_ar1d[24-1:0], siz_ar1d);
   mod_typ #(.TYP (bit [3:0][3:0])) mod_ar2d (clk, cnt_ar2d[16-1:0], siz_ar2d);

endmodule // test


module mod_typ #(
   parameter type TYP = byte
)(
   input  logic clk,
   output TYP   cnt = 0,
   output int   siz
);

   always @ (posedge clk)
   cnt <= cnt + 1;

   assign siz = $bits (cnt);

endmodule
