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


-- Test for concatenation of function call results.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity concat_func is
  port(in_word : in std_logic_vector(7 downto 0);
       out_word : out std_logic_vector(7 downto 0));
end entity concat_func;

architecture test of concat_func is
begin
  process(in_word)
    variable tmp  : unsigned(7 downto 0);
    variable int : integer;
  begin
    tmp := unsigned(in_word);
    int := to_integer(tmp);
    out_word <= in_word(7 downto 6) & std_logic_vector(to_unsigned(int, 3)) & std_logic_vector(resize(tmp, 3));
  end process;
end architecture test;
