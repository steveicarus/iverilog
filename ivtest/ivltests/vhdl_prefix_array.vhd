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


-- Example to test prefix for VTypeArray (and using function as index).

library ieee;
use ieee.std_logic_1164.all;

entity prefix_array is
  port(sel_word : in std_logic_vector(1 downto 0);
       out_word : out integer);
end entity prefix_array;

architecture test of prefix_array is
  type t_timeouts is
  record
    a : integer;
    b : integer;
  end record;

  type t_timeouts_table is array (natural range <>) of t_timeouts;

  constant c_TIMEOUTS_TABLE : t_timeouts_table(3 downto 0) :=
                              (0 => (a => 1, b => 2),
                               1 => (a => 3, b => 4),
                               2 => (a => 5, b => 6),
                               3 => (a => 7, b => 8));

begin
  process(sel_word)
  begin
    out_word <= to_unsigned((c_TIMEOUTS_TABLE(to_integer(unsigned(sel_word))).a), 32);
  end process;
end architecture test;
