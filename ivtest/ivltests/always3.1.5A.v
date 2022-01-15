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
//  SDW - Validate always if ( constant) statement ;
//  D:    Note that initial has to be before always to execute!

module main ;

reg [3:0] value1 ;

initial
	begin
           value1 = 0;
           # 5 ;
           if(value1 != 4'd4)
                $display("FAILED - always 3.1.5A always if ( constant) statement  \n");
           else
                $display("PASSED");
	   $finish;
        end

always if( 1'b1) begin
                   # 1;
                   value1 = value1 + 1;
                 end

endmodule
