//
// Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
//  Test the divide (/) operator

module top () ;

   reg [7:0] a, b, result;
   wire [7:0] wa, wb, wresult;

   assign wa = a;
   assign wb = b;
   assign wresult = wa / wb;

always @(a or b)
   result = a / b;

initial begin
   #1 a = 0; b = 1;
   # 1;
   if( result !== 8'b0)
     begin
        $display("FAILED - Divide 0/1 reg assign failed - is %b",result);
        $finish;
     end
   if( wresult !== 8'b0)
     begin
        $display("FAILED - Divide 0/1 wire assign failed - is %b",wresult);
        $finish;
     end

   #1 a = 1;
   #1 if( result !== 8'b1)
     begin
        $display("FAILED - Divide 1/1 reg assign failed - is %b",result);
	$finish;
     end
   if( wresult !== 8'b1)
     begin
        $display("FAILED - Divide 1/1 wire assign failed - is %b",wresult);
        $finish;
     end

   #1 a = 5; b = 2;
   #1 if( result !== 8'd2)
     begin
        $display("FAILED - Divide 5/2 reg assign failed - is %b",result);
        $finish;
     end
   if( wresult !== 8'd2)
     begin
        $display("FAILED - Divide 5/2 wire assign failed - is %b",wresult);
        $finish;
     end

   #1 a = 8'd255; b = 5;
    #1 if( result !== 51)
      begin
        $display("FAILED - Divide 255/5 reg assign failed - is %b",result);
        $finish;
      end
    if( wresult !== 51)
      begin
        $display("FAILED - Divide 255/5 wire assign failed - is %b",wresult);
        $finish;
      end

    #1 a = 1'bx; b = 3;
    #1 if( result !== 8'bxxxx_xxxx)
      begin
         $display("FAILED - Divide x/3 reg assign failed - is %b",result);
         $finish;
      end
   if( wresult !== 8'bxxxx_xxxx)
     begin
        $display("FAILED - Divide x/3 wire assign failed - is %b",wresult);
        $finish;
     end

   $display("PASSED");

end

endmodule
