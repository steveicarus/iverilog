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
//  SDW - Validate case with labels of x and z. Should be match

module main ();

reg error;
reg [2:0] val1,val2;
reg [2:0] result ;

always @( val1 )
  case (val1)
    3'b000: result = 0;
    3'b001: result = 1 ;
    3'b010: result = 2;
    3'bx11: result = 4;
    3'bz11: result = 5;
  endcase

initial
  begin
    error = 0;
    #1;
    val1 = 3'b0;
    #1;
    if(result !== 0)
      begin
        $display("FAILED case 3.8D - case (expr) lab1: ");
        error = 1;
      end
    #1;
    val1 = 3'b001;
    #1;
    if(result !== 1)
      begin
        $display("FAILED case 3.8D - case (expr) lab2: ");
        error = 1;
      end

    #1 ;
    val1 = 3'b010;
    #1;
    if(result !== 2)
      begin
        $display("FAILED case 3.8D - case (expr) lab3: ");
        error = 1;
      end
    #1 ;
    val1 = 3'bz11;
    #1;
    if(result !== 5)
      begin
        $display("FAILED case 3.8D - case (expr) lab5: ");
        error = 1;
      end

    #1 ;
    val1 = 3'bx11;
    #1;
    if(result !== 4)
      begin
        $display("FAILED case 3.8D - case (expr) lab4: ");
        error = 1;
      end

    if(error == 0)
      $display("PASSED");
  end

endmodule // main
