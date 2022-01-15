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
// Foundation, Inc., 59 Temple Place / Suite 330, Boston, MA 02111/1307, USA


// Example to test prefix for VTypeArray (and using function as index).

module test_prefix_aray();
  logic [1:0] sel_word;
  logic [31:0] out_word;

  prefix_array dut(sel_word, out_word);

  initial begin
    sel_word = 2;
    #1;

    if(out_word !== 32'd5) begin
        $display("FAILED out_word = %d", out_word);
        $finish();
    end

    $display("PASSED");
  end
endmodule
