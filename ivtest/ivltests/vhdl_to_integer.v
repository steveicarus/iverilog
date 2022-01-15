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


// Test for to_integer() function.

module to_int_test;
logic unsigned [7:0] unsign;
logic signed [7:0] sign;
to_int dut(unsign, sign);

initial begin
  unsign = 8'b11001100;
  sign = 8'b11001100;

  #1;

  if(dut.s_natural !== 204)
  begin
    $display("FAILED 1");
    $finish();
  end

  if(dut.s_integer !== -52)
  begin
    $display("FAILED 2");
    $finish();
  end

  $display("PASSED");
end
endmodule

