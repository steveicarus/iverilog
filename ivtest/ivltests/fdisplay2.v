/* Copyright (C) 2000 Stephen G. Tell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */
/* fdisplay2 - test $fopen and $fdisplay system tasks */

module fdisplay2;

   integer fp, dfp;
   reg [7:0] a;

   initial begin
      fp = $fopen("work/fdisplay2.out");
      if(fp != 2 && fp != 4 && fp != 8 && fp != 16 && fp != 32 && fp != 64)
         begin
	          $display("FAILED fopen fp=%d", fp);
	          $finish;
         end

      $fwrite(fp, "hello, world\n");
      a = 8'hac;

      //$fdisplay(1|fp, "a = %h; x: %b\n", a, a^8'h0f);
      dfp = 1|fp;
      $fdisplay(dfp, "a = 'h%h = 'b%b", a, a);

      $finish;
   end // initial begin

endmodule
