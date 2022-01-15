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
//  SDW - Validate release statement
//
// D: This code verifies the release statement.
// D: It depends on the force statement being
// D: functional! (Kinda have to - no way to
// D: release if you haven't forced the issue.
// D: It is intended to be self checking.
//
// By: Steve Wilson
//

module main ();

reg working;
reg timer;

initial
  working = 1;

initial
  begin
    #5 ;
    force working = 0;
  end

initial
  begin
    #10;
    release working;	// This releases the force
    #2 ;
    working = 1;	// This allows a new value onto the reg.
  end

initial
  begin
    #20;
    if(!working)
        $display("FAILED\n");
    else
        $display("PASSED\n");
  end

endmodule
