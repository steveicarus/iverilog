//
 // Copyright (c) 1999 Stephen Williams <steve@icarus.com>
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


 // time   value1   value2
 //         xxxx     xxxx
 //   1     0000
 //   2              0000
 //   3     0001
 //   4              0001
 //   5     0010
 //   6              0010

module main ;

reg [3:0] value1,value2,value3;

initial begin
   #1 ;
   #2 if (value2 != 0) begin
      $display("FAILED == at time 3, %b != 0", value2);
      $finish;
   end
   #2 if (value2 != 1) begin
      $display("FAILED == at time 5, %b != 1", value2);
      $finish;
   end
   #2 if (value2 != 2) begin
      $display("FAILED == at time 7, %b != 2", value2);
      $finish;
   end
   $display("PASSED");
   $finish;
end

initial begin
   #1 value1 = 4'b0000;
   #2 value1 = 4'b0001;
   #2 value1 = 4'b0010;
end

always  case (value1)
	  4'b0000 : begin
	     value2 = 4'b0000 ;
	     #2 ;
	  end
	  4'b0001 : begin
	     value2 = 4'b0001 ;
	     #2 ;
	  end
	  4'b0010 : begin
	     value2 = 4'b0010 ;
	     #2 ;
	  end
	  4'bxxxx : #2 ;

	  default : begin
	     $display("FAILED -- unexpected value1===%b", value1);
	     $finish;
	  end

	endcase


endmodule
