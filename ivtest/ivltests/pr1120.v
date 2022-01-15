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
//    for3.16A -  Template 1 - for(val1=0; val1 <= expr ; val1 = val1 + 1) some_action
//
module pr1120 ();

wire  [31:0] foo;
reg  [31:0] bar;

// FAIL
assign foo[31:16] = (bar & 32'hffffffff) >> 16;
// PASS
//assign foo[31:16] = bar >> 16;

initial
  begin
    bar = 32'ha5a5_3f3f;
    #100;
    $display("foo[31:16] = %x bar = %x",foo[31:16],bar);
    //if(foo[31:16]==((bar & 32'hffffffff) >> 16))
    if(foo[31:16] === 16'ha5a5)
	$display("PASS (%x)",foo[31:16]);
     else
       $display("FAIL (%x vs %x)",foo[31:16],((bar & 32'hffffffff) >> 16));
    $finish;
  end
endmodule
