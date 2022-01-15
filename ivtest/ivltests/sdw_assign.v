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
//
// SDW - Validation of assign construct
//
// D: Create a 32 bit vector and assign succesive values to
// D: the Right hand side expression. Verify the result is
// D: correct.
//

module main ();

wire [31:0] result;
reg  [31:0] a,b ;

assign result = a | b;

initial	// Excitation block
  begin
    a = 0;
    b = 0;
    # 5;
    a = 32'haaaaaaaa ;
    # 5;
    b = 32'h55555555 ;
    # 5 ;
  end

initial // Validation block
  begin
    # 1;
    if(result != 32'h0)
      begin
        $display("FAILED - result not initialized\n");
        $finish ;
      end

    # 5;
    if(result != 32'haaaaaaaa)
      begin
        $display("FAILED - result not updated\n");
        $finish ;
      end

    # 5;
    if(result != 32'hffffffff)
      begin
        $display("FAILED - result not updated\n");
        $finish ;
      end

    $display("PASSED\n");
    $finish ;
  end

endmodule
