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
//  SDW - Validate always casez ( reg_value) case_item1; case_item2;  case_item3; endcase
//  D:

module main ;

reg [3:0] value1,value2,value3;

initial
	begin
           #0;
//           $dumpfile("test.vcd");
//           $dumpvars(0,main);
           value3 = 0;
           #3 ;					// t=3
           value1 = 4'b0000 ;	// Picked up at time 4
           #5 ;				    // check at time 8
           $display("check == 0000:at time=%t value2=%h",$time,value2);
           if(value2 != 4'b0)
             begin
                $display("FAILED - always3.1.6C - casez 0 at %t",$time);
                value3 = 1;
             end
           #1 ;					// Picked up at time 10
           value1 = 4'b00z1 ;	// Set at time 9.

           #5 ;					// Check at time 14
           $display("check == 0001:at time=%t value2=%h",$time,value2);
           if(value2 != 4'b0001)
             begin
                $display("FAILED - always3.1.6C - casez z1 at %t",$time);
                value3 = 1;
             end
           #1;					// Picked up at time 16
           value1 = 4'b0100;	// Changed at time 15.

           #5;					// Check at time 20...
           $display("check == 0010:at time=%t value2=%h",$time,value2);
           if(value2 != 4'b0010)
             begin
                $display("FAILED - always3.1.6C - casez 4 at %t",$time);
                value3 = 1;
             end

           #10;
           if(value3 == 0)
              $display("PASSED");
	   $finish;
        end

always
       begin
         $display("Entering case at time=%t value1=%b",$time,value1);
         casez (value1)
               4'b0000: begin
                            #3 ;
                            value2 = 4'b0000 ;
                            $display("case0000: at time=%t",$time);
                            #3 ;
                        end
               4'b00z1: begin
                            #3 ;
                            value2 = 4'b0001 ;
                            $display("case00z1: at time=%t",$time);
                            #3 ;
                        end
               4'b0100: begin
                            #3 ;
                            value2 = 4'b0010 ;
                            $display("case100: at time=%t",$time);
                            #3 ;
                        end
                default:
                        begin
                         #2 ;
                          $display("default: %t",$time);
                        end
         endcase
         $display("Leaving case at time=%t",$time);
      end

endmodule
