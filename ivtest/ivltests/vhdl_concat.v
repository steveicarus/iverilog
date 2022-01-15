// Copyright (c) 2014 CERN
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


// Test for expression concatenation in VHDL.

module concat_test;
concat dut();

initial begin
  #1;       // wait for signal assignments

  if(dut.concat1 !== 2'b10)
  begin
    $display("FAILED: concat1 should be 10 but is %b", dut.concat1);
    $finish();
  end

  if(dut.concat2 !== 5'b11010)
  begin
    $display("FAILED: concat2 should be 11010 but is %b", dut.concat2);
    $finish();
  end

  $display("PASSED");
end
endmodule
