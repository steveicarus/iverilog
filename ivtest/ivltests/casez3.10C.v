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
//  SDW - Validate casez/endcase w/ z in label - no match case

module main ();

reg error;
reg [2:0] val1,val2;
reg [2:0] result ;

always @( val1 )
  casez (val1)
    5'b0000z: result = 0;
    5'b001z0: result = 1 ;
    5'b01zz0: result = 2;
  endcase

initial
  begin
    error = 0;

    val1 = 5'b0000z ;
    if(result !=0)
      begin
        $display("FAILED casez 3.10C - case (expr) lab1: ");
        error = 1;
      end

    val1 = 5'b001z0;
    if(result !=1)
      begin
        $display("FAILED casez 3.10C - case (expr) lab2: ");
        error = 1;
      end

    val1 = 5'b1zzzz;	// Should get no-action - expr = 3'b011
    if(result !=1)
      begin
        $display("FAILED casez 3.10C - case (expr) lab1: ");
        error = 1;
      end

    if(error == 0)
      $display("PASSED");
  end

endmodule // main
