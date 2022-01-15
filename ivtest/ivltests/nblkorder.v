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
// SDW - Validates Non-blocking order determinism IEEE1364-Draft, page 5-3,
// SDW - section 5.4.1.

module main ();

reg x,clock;
reg inval;
reg error;

always @(posedge clock)
  begin
    x <= ~inval;
    x <= inval;
  end

initial
  begin
    clock = 0;
    error = 0;
    #1;
    inval = 0;
    #5 ;
    clock = 1;
    #1 ;
    if(x !== inval)
      begin
        $display("FAILED - parallel non-blocking assign s/b 0, is %b",x);
        error = 1;
      end
    #6
    clock = 0;
    #1 ;
    inval = 1;
    #5 ;
    clock = 1;
    #1 ;
    if(x !== inval)
      begin
        $display("FAILED - parallel non-blocking assign s/b 1, is %b",x);
        error = 1;
      end
    #1 ;
    if(error == 0)
       $display("PASSED");
  end
endmodule
