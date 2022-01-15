// Copyright (c) 2016 CERN
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


// Power and division remainder operators test

module vhdl_pow_rem_test;
integer a, b, pow_res, rem_res;
vhdl_pow_rem dut(a, b, pow_res, rem_res);

initial begin
    a = 5;
    b = 2;

    if(pow_res != 25 || rem_res != 1) begin
        $display("FAILED 1");
        $finish();
    end

    a = -5;
    b = 3;

    if(rem_res != -2 || pow_res != -125) begin
        $display("FAILED 2");
        $finish();
    end

    $display("PASSED");
end

endmodule
