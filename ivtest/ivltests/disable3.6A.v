//
// Copyright (c) 1999 Steven Wilson (stevew@home.com)
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
//  SDW - Validate defparam

module main ();

reg clock;
reg q;
reg reset;
reg error;

always @(posedge clock or posedge reset)
  begin : FF
    # 2;
    if(reset)
      q <= 0;
    else
      q <= ~q;
  end

initial
  begin

    // Set reset to init f/f.
    error = 0;
    clock = 0;
    reset = 1;

    #4 ;
    if(q != 1'b0)
      begin
        $display("FAILED - disable3.6A - Flop didn't clear on clock & reset");
        error = 1;
      end
    reset = 1'b0;

    clock = 1'b1;
    # 3;
    if(q != 1'b1)
      begin
        $display("FAILED - disable3.6A - Flop didn't set on clock");
        error = 1;
      end

    clock = 1'b0;
    # 3;
    clock = 1'b1;	// Now cause the toggle edge
    # 1;
    disable FF;		// And disable the toggle event
    # 2;
    if(q != 1'b1)
      begin
        $display("FAILED - disable3.6A - Disable didn't stop FF toggle");
        error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule // main
