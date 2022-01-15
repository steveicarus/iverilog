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
//  SDW - Validate always casex ( reg_value) case_item1; case_item2;  case_item3; endcase
//  D:

module main ;

reg [3:0] value1,value2,value3;

initial
	begin
           #0;
           value3 = 0;
           #1 ;					// t=3
           value1 = 4'b0000 ;	// Picked up at time 6
           #9 ;				    // check at time 10
           if(value2 != 4'b0)
             begin
                $display("FAILED - always3.1.6B - casex 0 at %t",$time);
                value3 = 1;
             end
           #1 ;					// Picked up at time 12
           value1 = 4'b0011 ;	// Set at time 11.

           #5 ;					// Check at time 16
           if(value2 != 4'b0001)
             begin
                $display("FAILED - always3.1.6B - casex 1 at %t",$time);
                value3 = 1;
             end
           #1;					// Picked up at time 16
           value1 = 4'b0100;	// Changed at time 15.

           #5;					// Check at time 20...
           if(value2 != 4'b0010)
             begin
                $display("FAILED - always3.1.6B - casex 2 at %t",$time);
                value3 = 1;
             end

           #10;
           if(value3 == 0)
              $display("PASSED");
	   $finish;
        end

always  casex (value1)
               4'b0000: begin
                            #3 ;
                            value2 = 4'b0000 ;
                            #3 ;
                        end
               4'b00x1: begin
                            #3 ;
                            value2 = 4'b0001 ;
                            #3 ;
                        end
               4'b0100: begin
                            #3 ;
                            value2 = 4'b0010 ;
                            #3 ;
                        end
         endcase


endmodule
