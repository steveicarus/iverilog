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


-- std_logic values test.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_logic is
    port (low, high, hiz, dontcare, uninitialized, unknown : out std_logic);
end vhdl_logic;

architecture test of vhdl_logic is begin
    low <= '0';
    high <= '1';
    hiz <= 'Z';
    dontcare <= '-';
    uninitialized <= 'U';
    unknown <= 'X';
end architecture test;
