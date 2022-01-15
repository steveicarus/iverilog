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
//  SDW - Validate always reg_lvalue <= @ (event_expression)  constant
//

module main ;

reg [3:0] value1 ;
reg event_var ;

initial
begin
  # 2 ;
  value1 = 5'h 0 ;
  # 3 ;
  event_var = 1'b0 ;
  # 2 ;
  value1 = 5'h 0 ;
  # 3 ;
  event_var = 1'b1 ;
  #5 ;
end

initial
  begin			// Should be xxxx at initial time
    if(value1 !== 4'bxxxx)
	$display("FAILED - always reg_lvalue <= @ (event_expression) constant \n");
    # 6 ;
    if(value1 != 4'h5)	// Time 5 should see a change of 0 to 1
	$display("FAILED - always reg_lvalue <= @ event_identifier boolean_expression\n");
    # 5 ;
    if(value1 != 4'h5)	// Time 5 should see a change of 0 to 1
	$display("FAILED - always reg_lvalue <= @ event_identifier boolean_expression\n");
        begin
          $display("PASSED\n");
          $finish ;
        end
  end

always value1 <= @ (event_var)  4'h5 ;


endmodule
