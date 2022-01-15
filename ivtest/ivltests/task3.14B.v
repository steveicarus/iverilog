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
//  SDW - Validate task w/ simple input.


module main;

reg globvar;

task my_task ;
  input in1;
  globvar = in1;
endtask

initial
  begin
    globvar = 1'b0;
    my_task(1'b1);
    if(globvar)
      $display("PASSED");
    else
      $display("FAILED - task 3.14B task didn't correctly affect global var");
  end

endmodule // main
