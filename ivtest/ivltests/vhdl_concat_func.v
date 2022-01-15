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


// Test for concatenation of function call results.

module test_concat_func();
  logic [7:0] in_word, out_word;

  concat_func dut(in_word, out_word);

  initial begin
    in_word = 8'b11101010;
    #1;

    if(out_word !== 8'b11010010) begin
        $display("FAILED out_word = %b", out_word);
        $finish();
    end

    $display("PASSED");
  end
endmodule
