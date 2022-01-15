//
// Copyright (c) 2003 Steve Williams
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

`timescale 1ns / 1ps

module main;

   task test;
      $display("time within task: %t", $time);
   endtask // test

   initial begin
      $timeformat(-9, 3, "ns", 6);
      #1 $display("time within module: %t", $time);
      test;
   end

endmodule
