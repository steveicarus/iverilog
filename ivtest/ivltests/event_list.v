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
//  Check support for event lists with named events.
//

module main ();

   reg flag1, flag2, flag12;
   event event_1, event_2;

   always @ event_1  flag1 = ~flag1;

   always @ event_2  flag2 = ~flag2;

   always @(event_1 or event_2) flag12 = ~flag12;

   initial begin
      flag1 = 0;
      flag2 = 0;
      flag12 = 0;

      #1  -> event_1;

      #1
	if (flag1 !== 1) begin
	   $display("FAILED -- event_1 didn't trigger flag1");
	   $finish;
	end

        if (flag2 !== 0) begin
	   $display("FAILED -- event_1 DID trigger flag2");
	   $finish;
	end

        if (flag12 !== 1) begin
	   $display("FAILED -- event_1 didn't trigger flag12");
	   $finish;
	end

      flag1 = 0;
      flag2 = 0;
      flag12 = 0;

      #1  -> event_2;

      #1
	if (flag1 !== 0) begin
	   $display("FAILED -- event_2 DID trigger flag1");
	   $finish;
	end

        if (flag2 !== 1) begin
	   $display("FAILED -- event_2 didn't trigger flag2");
	   $finish;
	end

        if (flag12 !== 1) begin
	   $display("FAILED -- event_1 didn't trigger flag12");
	   $finish;
	end


      $display("PASSED");
   end

endmodule
