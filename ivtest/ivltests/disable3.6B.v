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
//  SDW - Validate defparam

module main ();

reg clock;
reg q;
reg reset;
reg error;

task task_a ;
  begin
    # 2;
    if(reset)
      q = 0;
    else
      q = ~q ;
  end
endtask

initial
  begin

    // Set reset to init f/f.
    error = 0;
    reset = 1;
    task_a ;	// Same as posedge reset in previous test

    #4 ;
    if(q != 1'b0)
      begin
        $display("FAILED - disable3.6B - Flop didn't clear on clock & reset");
        error = 1;
      end
    reset = 1'b0;

    task_a ;	// First clock edge from orig test
    # 3;
    if(q != 1'b1)
      begin
        $display("FAILED - disable3.6B - Flop didn't set on clock");
        error = 1;
      end

    # 3;

    fork
       task_a ;		// Toggle f/f clock edge

       begin
         # 1;
         disable task_a;	// And disable the toggle event
       end
    join

    if(q != 1'b1)
      begin
        $display("FAILED - disable3.6B - Disable task didn't stop toggle");
        error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule // main
