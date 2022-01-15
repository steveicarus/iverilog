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
//  SDW - Validate always block instantiation.
//
//  D: Test validate others versions of always block
//  D: including posedge, negedge.
//
//

module main ();
reg working;
reg clock ;

initial	// Used to generate timing of events
  begin
     working = 0;
     clock = 0;
     #4 ;
     working = 0;
     #1 ;
     clock = 1;	// 1ns between setting working and clock edge.
     #4 ;
     working = 0;
     #1 ;
     clock = 0;	// 1ns between setting working and clock edge.
     #5  ;
  end

always #2
  working = 1 ;

initial // This is the validation block
  begin
    # 3;	// Check #2 always at 3ns
    if(!working)
      begin
        $display("FAILED - delayed always\n");
        $finish ;
      end
    # 3;	// Check posedge at 6 ns
    if(!working)
      begin
        $display("FAILED - posedge always\n");
        $finish ;
      end
    # 7;	// Check negedge at 11ns
    if(!working)
      begin
        $display("FAILED - posedge always\n");
        $finish ;
      end
    $display("PASSED\n");
    $finish;
  end

always @ (posedge clock)
    working = 1;

always @ (negedge clock)
    working = 1;

endmodule
