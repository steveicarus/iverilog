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
//  SDW - always release net_lvalue ;
//  D:    No dependancy

module main ;

wire [3:0] value1 ;

initial
  begin
    #15;
    if(value1 != 4'h5)
      $display("FAILED - 3.1.3H always release net_lvalue;\n");
    else
	begin
           $display("PASSED\n");
	   $finish;
        end
  end

always release value1 ;

endmodule
