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


// Test for 'range, 'reverse_range, 'left and 'right attributes in VHDL.

module range_test;
range_entity dut();

initial begin
  int i;
  #1;       // wait for signal assignments

  if(dut.left_asc !== 2)
  begin
    $display("FAILED: left_asc should be %2d but is %2d", 2, dut.left_asc);
    $finish();
  end

  if(dut.right_asc !== 4)
  begin
    $display("FAILED: right_asc should be %2d but is %2d", 2, dut.right_asc);
    $finish();
  end

  if(dut.left_dsc !== 9)
  begin
    $display("FAILED: left_dsc should be %2d but is %2d", 2, dut.left_dsc);
    $finish();
  end

  if(dut.right_dsc !== 3)
  begin
    $display("FAILED: right_dsc should be %2d but is %2d", 2, dut.right_dsc);
    $finish();
  end

  if(dut.pow_left !== 16)
  begin
    $display("FAILED: pow_left should be %2d but is %2d", 16, dut.pow_left);
    $finish();
  end

  if(dut.rem_left !== 2)
  begin
    $display("FAILED: rem_left should be %2d but is %2d", 2, dut.rem_left);
    $finish();
  end

  for(i = $left(dut.ascending); i <= $right(dut.ascending); i++)
  begin
    if(2*i !== dut.ascending[i])
    begin
        $display("FAILED: ascending[%2d] should be %2d but is %2d", i, 2*i, dut.ascending[i]);
        $finish();
    end
  end

  for(i = $right(dut.descending); i <= $left(dut.descending); i++)
  begin
    if(3*i !== dut.descending[i])
    begin
        $display("FAILED: descending[%2d] should be %2d but is %2d", i, 3*i, dut.descending[i]);
        $finish();
    end
  end

  for(i = $left(dut.ascending_rev); i <= $right(dut.ascending_rev); i++)
  begin
    if(4*i !== dut.ascending_rev[i])
    begin
        $display("FAILED: ascending_rev[%2d] should be %2d but is %2d", i, 4*i, dut.ascending_rev[i]);
        $finish();
    end
  end

  for(i = $right(dut.descending_rev); i <= $left(dut.descending_rev); i++)
  begin
    if(5*i !== dut.descending_rev[i])
    begin
        $display("FAILED: descending_rev[%2d] should be %2d but is %2d", i, 5*i, dut.descending_rev[i]);
        $finish();
    end
  end

  $display("PASSED");
end
endmodule
