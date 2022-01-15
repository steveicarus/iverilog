// Copyr (c) 2015-2016 CERN
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


// Test for shift operators (logical and arithmetic)

module shifter_test;
reg signed [7:0] inp, out_srl, out_sll, out_sra, out_sla, out_shl_u, out_shr_u, out_shl_s, out_shr_s;
shifter dut(inp, out_srl, out_sll, out_sra, out_sla, out_shl_u, out_shr_u, out_shl_s, out_shr_s);

initial begin
  inp = 8'b11101100;
  #1;       // wait for signal assignments

  if(out_srl !== 8'b01110110)
  begin
    $display("FAILED 1");
    $finish();
  end

  if(out_sll !== 8'b11011000)
  begin
    $display("FAILED 2");
    $finish();
  end

  if(out_sra !== 8'b11110110)
  begin
    $display("FAILED 3");
    $finish();
  end

  if(out_sla !== 8'b11011000)
  begin
    $display("FAILED 4");
    $finish();
  end

  if(out_shl_u !== 8'b10110000)
  begin
    $display("FAILED 5");
    $finish();
  end

  if(out_shr_u !== 8'b00111011)
  begin
    $display("FAILED 6");
    $finish();
  end

  if(out_shl_s !== 8'b10110000)
  begin
    $display("FAILED 7");
    $finish();
  end

  if(out_shr_s !== 8'b11111011)
  begin
    $display("FAILED 8");
    $finish();
  end

  $display("PASSED");
end
endmodule

