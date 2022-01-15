-- Copyright (c) 2014 CERN
-- Maciej Suminski <maciej.suminski@cern.ch>
--
-- This source code is free software; you can redistribute it
-- and/or modify it in source code form under the terms of the GNU
-- General Public License as published by the Free Software
-- Foundation; either version 2 of the License, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA


-- Test for 'range, 'reverse_range, 'left and 'right attributes in VHDL.

library ieee;
use ieee.std_logic_1164.all;
use work.vhdl_range_pkg.all;

entity range_entity is
    port (gen_vals: in std_logic);
end range_entity;

architecture test of range_entity is
  type int_array is array (integer range <>) of integer;

  signal ascending : int_array(2 to 4);
  signal descending : int_array(9 downto 3);
  signal ascending_rev : int_array(8 to 13);
  signal descending_rev : int_array(15 downto 10);
  signal range_pow : int_array(2**4 downto 0);
  signal range_rem : int_array(8 rem 3 downto 0);
  signal left_asc, right_asc, left_dsc, right_dsc, pow_left, rem_left : integer;

  -- There is no limited ranged integer in SystemVerilog, so just see if it compiles
  signal int_asc : integer_asc;
  signal int_desc : integer_desc;
begin
  process(gen_vals) begin
    left_asc <= ascending'left;
    right_asc <= ascending'right;
    left_dsc <= descending'left;
    right_dsc <= descending'right;
    pow_left <= range_pow'left;
    rem_left <= range_rem'left;

    -- 'range test
    for i in ascending'range loop
        ascending(i) <= i * 2;
    end loop;

    for i in descending'range loop
        descending(i) <= i * 3;
    end loop;

    -- 'reverse_range test
    for i in ascending_rev'reverse_range loop
        ascending_rev(i) <= i * 4;
    end loop;

    for i in descending_rev'reverse_range loop
        descending_rev(i) <= i * 5;
    end loop;
  end process;
end test;
