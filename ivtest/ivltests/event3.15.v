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
//  SDW - Event list_of_event_identifiers, -> event_identifier.
//

module main ();

reg [31:0] v1,v2,v3;
reg error;
event event_1, event_2;

always @ event_1
  begin
     v1 = v1 + 1;
  end

always @ event_2
  begin
     v2 = 1;
  end

initial
  begin
    error = 0;
    v1 = 0;
    v2 = 0;
    v3 = 0;
//    $dumpfile("test.vcd");
//    $dumpvars(0,main);

    #(5);
    -> event_1;
    v3 = 1;
    #1 ;		  // Need delay here or race with always schedule
    if(v1 !== 1)
      begin
        $display("FAILED - event3.15 event1 trigger didn't occur");
        error = 1;
      end

    #5 -> event_2;
    #1 ;
    if(v2 !== 1)
      begin
        $display("FAILED - event3.15 event2 trigger didn't occur");
        error = 1;
      end
    v3 = 2;

    #5 -> event_1;
    #1 ;
    if(v1 !== 2)
      begin
        $display("FAILED - event3.15 event1 trigger didn't occur");
        error = 1;
      end
    v3 = 3;
    #5 ;

    if(error === 0)
       $display("PASSED");
  end

endmodule
