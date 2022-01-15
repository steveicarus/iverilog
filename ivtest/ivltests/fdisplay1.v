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
/* fdisplay1 - test $fwrite and $fdisplay system tasks without using $fopen
 *
 * NB: this may need a little tweaking, as I'm not sure that all verilogs
 * have the predefined $fdisplay descriptors 2 and 3 matching what
 * vpi_mcd_printf provides.
 */

module fdisplay1;

   integer fp;
   reg [7:0] a;

   initial begin

      $display("message to stdout (from $display)\n");
      $fwrite(1, "another message (via fwrite)  ");
      $fdisplay(1,"to stdout\n (via fdisplay)");
      #5

      a = 8'h5a;
      $fwrite(1, "a = %b at %0t\n", a, $time);

      $finish(0);

   end // initial begin

endmodule
