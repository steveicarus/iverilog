// Copyright (c) 2015 CERN
// @author Maciej Suminski <maciej.suminski@cern.ch>
//
// This source code is free software; you can redistribute it
// and/or modify it in source code form under the terms of the GNU
// General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA


// 'wait on' & 'wait until' test

module vhdl_wait_test;
logic [1:0] a, b;
vhdl_wait dut(a, b);

always @(posedge b[0]) begin
    $display("wait 1 acknowledged");
    // complete "wait 2"
    a[1] = 1'b0;
end

always @(posedge b[1]) begin
    $display("wait 2 acknowledged");
end

initial begin
    // complete "wait 1"
    a = 2'b00;
end

endmodule
