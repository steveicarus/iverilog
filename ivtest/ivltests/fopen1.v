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
// SDW - reworked a bit to account for the fact that it HAS to be

/* fopen1 - test $fopen system task */

module fopen1;
   reg [31:0]  fp;
   reg error ;

   initial begin

      fp = $fopen("work/fopen1.out");
      case(fp)
        32'h0000_0001: error = 1;
        32'h0000_0002: error = 0;
        32'h0000_0004: error = 1;
        32'h0000_0008: error = 1;
        32'h0000_0010: error = 1;
        32'h0000_0020: error = 1;
        32'h0000_0040: error = 1;
        32'h0000_0080: error = 1;
        32'h0000_0100: error = 1;
        32'h0000_0200: error = 1;
        32'h0000_0400: error = 1;
        32'h0000_0800: error = 1;
        32'h0000_1000: error = 1;
        32'h0000_2000: error = 1;
        32'h0000_4000: error = 1;
        32'h0000_8000: error = 1;
        32'h0001_0000: error = 1;
        32'h0002_0000: error = 1;
        32'h0004_0000: error = 1;
        32'h0008_0000: error = 1;
        32'h0010_0000: error = 1;
        32'h0020_0000: error = 1;
        32'h0040_0000: error = 1;
        32'h0080_0000: error = 1;
        32'h0100_0000: error = 1;
        32'h0200_0000: error = 1;
        32'h0400_0000: error = 1;
        32'h0800_0000: error = 1;
        32'h1000_0000: error = 1;
        32'h2000_0000: error = 1;
        32'h4000_0000: error = 1;
        32'h8000_0000: error = 1;
        default: error = 1;		// std_io!
      endcase

      $display("fp = %b",fp);

      if(error == 0)
        $display("PASSED");
      else
        $display("FAILED");
	  $fclose(fp);

      $finish;
   end
endmodule
