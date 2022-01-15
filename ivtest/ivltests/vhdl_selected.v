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


// Test for selected assignments.

module vhdl_selected_test;
logic [1:0] sel;
logic [3:0] in;
logic out;
vhdl_selected dut(sel, in, out);

initial begin
    in = 4'b1010;

    sel = 1'b00;
    #1;
    if(out !== 1'b0) begin
        $display("FAILED 1");
        $finish();
    end

    sel = 1'b01;
    #1;
    if(out !== 1'b1) begin
        $display("FAILED 2");
        $finish();
    end

    sel = 1'b10;
    #1;
    if(out !== 1'b0) begin
        $display("FAILED 3");
        $finish();
    end

    sel = 1'b11;
    #1;
    if(out !== 1'b1) begin
        $display("FAILED 4");
        $finish();
    end

    $display("PASSED");
end
endmodule
