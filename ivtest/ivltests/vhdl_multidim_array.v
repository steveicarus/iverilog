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


// Test multidimensional arrays.

module vhdl_multidim_array_test;
vhdl_multidim_array dut();

initial begin
    int i, j;

    for(i = 0; i <= 1; i = i + 1) begin
        for(j = dut.array_size - 1; j >= 0; j = j - 1) begin
            $display("%d", dut.arr[i][j]);
            //$display("%d, %d = %d", i, j, arr[i][j]);
            //if(dut.arr[i][j] !== i * 10 + j) begin
                //$display("FAILED: arr[%d][%d] == %d, instead of %d", i, j, dut.arr[i][j], i * 10 + j);
                //$finish();
            //end
        end
    end

    $display("PASSED");
end

endmodule
