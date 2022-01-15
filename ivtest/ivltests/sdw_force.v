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
// SDW - Force stmt validation
//
// D: This code verifies the force statement
// D: It is intended to be self checking.
//

module main ();

reg working;
reg timer;

initial
begin
  timer = 1;
  # 5;
  timer = 0;
  # 5 ;
  timer = 1;
  # 5 ;
end


initial
  begin
    working = 1;
    #2 ;  // Validate that force occurs
    force timer = 0;
    if( timer == 1) working = 0;
    #10 ; // Validate that force stays in effect
    if( timer == 1) working = 0;
  end

initial
  begin
    #20;
    if(!working)
        $display("FAILED\n");
    else
        $display("PASSED\n");
  end

endmodule
