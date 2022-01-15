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
// SDW - validate named blocks/disable stmt
//
// D: This code verifies both named blocks and the disable statement
// D: It is intended to be self checking.
//

module main ();

reg working;
reg timer;

initial
  begin:my_block
    working = 1;
    #5;
    working = 1;
    #5;
    working = 1;
    #5;
    working = 0;
    #5;
  end

initial
  begin
    #15;
    disable my_block;
  end

initial
  begin
    #20;
    if(!working)
       $display("FAILED");
    else
       $display("PASSED");
  end

endmodule
