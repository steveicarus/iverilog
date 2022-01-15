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


// Test for delayed assignment statements.

module vhdl_delay_assign_test;
logic a, b, c;
int passed = 0;
vhdl_delay_assign dut(a, b, c);

always @(b)
begin
    if($time == 10) begin
        passed = passed + 1;
    end else begin
        $display("FAILED 1");
        $finish();
    end
end

always @(c)
begin
    if($time == 10) begin
        passed = passed + 1;
    end else begin
        $display("FAILED 2");
        $finish();
    end
end

initial begin
    a = 1;
    #11;

    if(passed !== 2) begin
        $display("FAILED 3");
        $finish();
    end

    $display("PASSED");
end

endmodule
