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


-- Test case for handling an array of arrays

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vhdl_array_of_array is
end entity vhdl_array_of_array;

architecture test of vhdl_array_of_array is
  type t_byte_array is array (natural range <>) of std_logic_vector(7 downto 0);
  signal sig : t_byte_array(2 downto 0);

begin
    sig <= (0 => x"aa", 1 => x"bb", 2 => x"cc");
end architecture test;
