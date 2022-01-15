// Copyright (c) 2015 CERN
// Maciej Suminski <maciej.suminski@cern.ch>
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


// Test for bug reports:
// #943: VHDL enum values not available outside of switch statements
// #944: VHDL enum type declaration generates syntax errors

module bg943_test();
logic clk, rst, q;
e dut(clk, rst, q);

initial begin
    clk = 0;
    rst = 1;
    #1;

    clk = 1;
    #1;
    if(q !== 1'b0) begin
        $display("FAILED 1");
        $finish();
    end
    #1;

    clk = 0;
    rst = 0;
    #1;

    clk = 1;
    #1;
    if(q !== 1'b1) begin
        $display("FAILED 2");
        $finish();
    end

    $display("PASSED");
end
endmodule
