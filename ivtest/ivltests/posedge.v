//
// Copyright (c) 2000 Stephen Williams (steve@icarus.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//
//  single bit positive events
//

module main ();

   reg flag1;
   reg event_1;

   always @ (posedge event_1)  flag1 = ~flag1;

   initial begin
      event_1 = 1'b0;
      #1  flag1 = 0;

      #1  event_1 = 1'b1;

      #1
	if (flag1 !== 1'b1) begin
	   $display("FAILED -- 0->1 didn't trigger flag1");
	   $finish;
	end

      event_1 = 1'b0;

      #1
	if (flag1 !== 1'b1) begin
	   $display("FAILED -- 1->0 DID trigger flag1");
	   $finish;
	end

      $display("PASSED");
   end

endmodule
