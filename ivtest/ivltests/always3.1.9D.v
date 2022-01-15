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
//  SDW - Validate always disable block_identifier ;


module main ;


reg [3:0] value1 ;

always begin : block_id
          #4 ;
          value1 = 1;
          $finish ;
      end

initial
  begin
    value1 = 0;
    #5;
    if(value1 === 1'b0)
      $display("PASSED");
    else
      $display("FAILED - always3.1.9D always #1 disable block_id");
    #1;
    $finish ;
  end

always #3 disable block_id ;

endmodule
