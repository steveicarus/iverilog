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


// Test for vvp_net_fun_t::recv_vec4_pv() implementation (vvp).

module vvp_recv_vec4_pv (input wire logic clk, input wire logic inp,
                         output wire logic[16:0] arr_out);
logic[16:0] arr;

always begin
    arr[15:0] <= arr[16:1];
    @(clk); wait(clk == 1'b1);
end

assign arr[16] = inp;
assign arr_out = arr;

endmodule


module vvp_recv_vec4_pv_test;
logic clk, inp;
logic [16:0] arr, src;
vvp_recv_vec4_pv dut(clk, inp, arr);

always #5 clk <= ~clk;

initial begin
    int i;

    src <= 17'b01101110010011011;
    clk <= 1'b1;
    #5;

    for(i = 0; i < 17; i = i + 1) begin
        #10 inp = src[i];
    end

    #5; // wait for the last assignment occuring in the for loop above

    if(arr !== src) begin
        $display("FAILED");
    end else begin
        $display("PASSED");
    end

    $finish();
end

endmodule

