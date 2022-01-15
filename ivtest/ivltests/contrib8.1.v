// Copyright (C) 1999  Motorola, Inc.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// 9/7/99 - SDW - Added if(!a) FAILED else PASSED

module blahblah ();
   parameter foo = 1;

   reg [31:0] a;

   initial
     begin
       test(a);
       if(a != 1)
         $display("FAILED - contrib 8.1  foo not passed into task - bad scope");
       else
         $display("PASSED");
     end


   task test;
      output blah;
      begin
         blah = foo;
      end
   endtask // test

endmodule // blahblah
