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


-- Tests signal initializers.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_init is
end;

architecture test of vhdl_init is
    -- Convert string to bitstring in initalizer
    signal a : std_logic_vector(7 downto 0) := "11101001";
    -- Initialize with aggregate expression
    signal b : std_logic_vector(3 downto 0) := (0 => '0', 3 => '1', 1 => '1', 2 => '0');
    -- Initialize with aggregate expression, inverted range
    signal c : std_logic_vector(0 to 3) := (3 => '1', others => '0');
begin
    -- Architecture statement part cannot be empty
    -- Assign the previous value, otherwise you will get unknown value
    a <= "11101001";
end test;
