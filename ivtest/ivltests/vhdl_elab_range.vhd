-- Copyright (c) 2016 CERN
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


-- Bug report test (could not elaborate a function used to specify a range).

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vhdl_elab_range is
    port(sig_left, sig_right : out integer);
end entity vhdl_elab_range;

architecture test of vhdl_elab_range is
    function inc_by_two(a : in integer) return integer is
    begin
        return a + 2;
    end function inc_by_two;

  signal test_sig : unsigned(inc_by_two(0) downto 0) := (others => '0');
begin
  process
  begin
      sig_left <= test_sig'left;
      sig_right <= test_sig'right;
      wait;
  end process;
end architecture test;
