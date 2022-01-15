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
//  SDW - Validate always case ( reg_value) case_item1; case_item2;  case_item3; endcase
//  D:

module main ;

reg [3:0] value1,value2,value3;

initial
	begin
           #0;
           value3 = 0;
           #3 ;					// t=3
           value1 = 4'b0000 ;	// Picked up at time 4
           #5 ;				    // check at time 8
           if(value2 != 4'b0)
             begin
                $display("FAILED - always3.1.6A - case 0 at %t",$time);
                value3 = 1;
             end
           #1 ;					// Picked up at time 10
           value1 = 4'b0001 ;	// Set at time 9.

           #5 ;					// Check at time 14
           if(value2 != 4'b0001)
             begin
                $display("FAILED - always3.1.6A - case 1 at %t",$time);
                value3 = 1;
             end
           #1;					// Picked up at time 16
           value1 = 4'b0010;	// Changed at time 15.

           #5;					// Check at time 20...
           if(value2 != 4'b0010)
             begin
                $display("FAILED - always3.1.6A - case 2 at %t",$time);
                value3 = 1;
             end

           #10;
           if(value3 == 0)
              $display("PASSED");
	   $finish;
        end

always  case (value1)
               4'b0000: begin
                            #3 ;
                            value2 = 4'b0000 ;
                            #3 ;
                        end
               4'b0001: begin
                            #3 ;
                            value2 = 4'b0001 ;
                            #3 ;
                        end
               4'b0010: begin
                            #3 ;
                            value2 = 4'b0010 ;
                            #3 ;
                        end
                default: #2 ;
         endcase


endmodule
