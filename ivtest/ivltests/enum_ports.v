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


// Tests enum as a port type.

typedef enum integer { var_presence, var_identif, var_1, var_2, var_3, var_rst, var_4, var_5, var_whatever } t_var;

module enum_ports(input wire t_var var_i, output t_var var_o, output reg is_var_rst);

initial begin
    var_o = var_presence;
end

always @(var_i)
begin
    if(var_i == var_rst)
        is_var_rst = 1'b1;
    else
        is_var_rst = 1'b0;
end
endmodule

module test_unit();
t_var var_in, var_out;
reg result;

enum_ports dut(var_in, var_out, result);

initial begin
    #1;

    if(var_out !== var_presence) begin
        $display("FAILED 1");
        $finish();
    end

    var_in = var_1;
    #1;
    if(result !== 1'b0) begin
        $display("FAILED 2");
        $finish();
    end

    var_in = var_rst;
    #1
    if(result !== 1'b1) begin
        $display("FAILED 3");
        $finish();
    end

    $display("PASSED");
end

endmodule
