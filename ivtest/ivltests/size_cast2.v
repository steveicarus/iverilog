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


// Tests size casting of complex expressions.

module resize(output wire logic [4:0] result);
  logic [6:0] a;
  assign result = (5'(a + 2));

  initial begin
      a = 7'd39;
  end
endmodule

module resize_test();
  logic [4:0] result;
  resize dut(result);

  initial begin
    #1;

    if(result !== 5'd9) begin
        $display("FAILED");
        $finish();
    end

    $display("PASSED");
  end
endmodule
