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


// Test for accessing constant records & arrays of records in VHDL.

module vhdl_const_record_test;
int sel;
logic [7:0] hex;
logic [7:0] aval;
vhdl_const_record dut(sel, hex, aval);

initial begin
  if(dut.sig !== 8'h66)
  begin
    $display("FAILED 1");
    $finish();
  end

  sel = 0;
  #1;
  if(hex !== 8'h14 || aval !== 8'haa || dut.sig2 !== 8'h00)
  begin
    $display("FAILED 2");
    $finish();
  end

  sel = 1;
  #1;
  if(hex !== 8'h24 || aval !== 8'hbb || dut.sig2 !== 8'h11)
  begin
    $display("FAILED 3");
    $finish();
  end

  sel = 2;
  #1;
  if(hex !== 8'h34 || aval !== 8'hcc || dut.sig2 !== 8'h22)
  begin
    $display("FAILED 4");
    $finish();
  end

  sel = 3;
  #1;
  if(hex !== 8'h56 || aval !== 8'hdd || dut.sig2 !== 8'h33)
  begin
    $display("FAILED 5");
    $finish();
  end

  $display("PASSED");
end
endmodule

