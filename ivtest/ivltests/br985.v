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


module br985_test;
logic [3:0] sel;
logic value, spi_sdo;
bug3 dut(sel, value, spi_sdo);

initial begin
    int i;
    sel = 4'b0000;

    #1 if(spi_sdo !== 1'b1)
    begin
        $display("FAILED");
        $finish();
    end

    for(i = 1; i < 16; i = i + 1)
    begin
        sel = i;
        #1 if(spi_sdo !== 1'b0)
        begin
            $display("FAILED");
            $finish();
        end
    end

    $display("PASSED");
end

endmodule
