// Copyright (c) 2016 CERN
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


// Test for subtype definitions.

module vhdl_subtypes_test;
int a, b, c;
time d;
int e;

vhdl_subtypes dut(a, b, c, d, e);

initial
begin
    #1;

    if(a !== 1) begin
        $display("FAILED");
        $finish();
    end

    if(b !== 2) begin
        $display("FAILED");
        $finish();
    end

    if(c !== 3) begin
        $display("FAILED");
        $finish();
    end

    if(d !== 4) begin
        $display("FAILED");
        $finish();
    end

    if(e !== 5) begin
        $display("FAILED");
        $finish();
    end

    $display("PASSED");
end

endmodule
