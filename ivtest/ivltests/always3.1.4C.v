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
//  SDW - Validate always # delay_value reg_lvalue = boolean_expr ;
//  D:    Note that initial has to be before always to execute!

module main ;

reg [3:0] value1 ;
reg err ;

initial
	begin
           err = 0;
           # 1;
           if(value1 !== 4'bxxxx)
             begin
               $display("FAILED - 3.1.4C - initial value not xxxx;\n");
               err = 1;
             end
           #10 ;
           if(value1 != 4'h5)
             begin
               $display("FAILED - 3.1.4C - always # delay_value reg_lvalue = boolean_expr\n");
               err = 1;
             end
           #10 ;
           if(value1 != 4'hA)
             begin
               $display("FAILED - 3.1.4C - always # delay_value reg_lvalue = boolean_expr\n");
               err = 1;
             end

           if (err == 0)
             $display("PASSED\n");

           $finish;

        end

always # 10 value1 = ~value1;

endmodule
