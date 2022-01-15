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


// std_logic values test.

module vhdl_logic_test;

logic low, high, hiz, dontcare, uninitialized, unknown;
vhdl_logic dut(low, high, hiz, dontcare, uninitialized, unknown);

initial begin
    if(low !== 1'b0) begin
        $display("FAILED low");
        $finish();
    end

    if(high !== 1'b1) begin
        $display("FAILED high");
        $finish();
    end

    if(hiz !== 1'bz) begin
        $display("FAILED hiz");
        $finish();
    end

    if(dontcare !== 1'bx) begin
        $display("FAILED dontcare");
        $finish();
    end

    if(uninitialized !== 1'bx) begin
        $display("FAILED uninitialized");
        $finish();
    end

    if(unknown !== 1'bx) begin
        $display("FAILED unknown");
        $finish();
    end

    $display("PASSED");
end
endmodule
