-- Copyright (c) 2015 CERN
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


-- Test for 'range, 'left and 'right attributes in VHDL subprograms.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.range_func_pkg.all;

entity range_func is
end range_func;

architecture test of range_func is
  signal neg_inp : std_logic_vector(3 downto 0);
  signal neg_out : std_logic_vector(3 downto 0);
  signal rev_inp : std_logic_vector(3 downto 0);
  signal rev_out : std_logic_vector(3 downto 0);
begin
  neg_inp <= "1100";
  rev_inp <= "1000";

  process(neg_inp)
  begin
    neg_out <= negator(neg_inp);
  end process;

  process(rev_inp)
  begin
    rev_out <= reverse(rev_inp);
  end process;
end test;

