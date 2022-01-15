//
// Copyright (c) 2000 Steve Wilson (stevew@home.com)
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
//    function3.11E - Validate calling a task in a function causes an error.
//
module test ;

task foo2;
$display("insided foo2");
endtask

function [31:0] foo;
input [31:0] a;
foo = a;
foo2;
endfunction

reg [31:0] b;
initial
  begin
  $display("hi");
  b = foo(123);
 end

endmodule
