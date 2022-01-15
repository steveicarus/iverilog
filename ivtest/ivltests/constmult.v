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
//  SDW - Validate constant multiplication in array define.
//
//

module main ();

reg [5 * 2: 0] val1;
reg [10'h1 * 10: 0 ] val2 ;

initial
  begin
     val1 = 11'h1 * 5;
     val2 = 11'h2 * 4;
     if((val1 === 11'h5) && (val2 === 11'h8))
        $display("PASSED");
     else
        $display("FAILED");
  end

endmodule
