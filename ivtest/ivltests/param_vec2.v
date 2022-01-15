/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* Based on PR#557. */

module test ();

    parameter   s_ack           = 3;

    parameter [s_ack-1:0]
        Ack_Wait = 2'b 00,
        Ack_Rdy  = 2'b 11,
        Ack_Err  = 2'b 10;

   initial begin
      if ($bits(Ack_Wait) != 3) begin
	 $display("FAILED -- $bits(Ack_Wait) == %0d (should be 3)",
		  $bits(Ack_Wait));
	 $finish;
      end

      $display("PASSED");
   end

endmodule
