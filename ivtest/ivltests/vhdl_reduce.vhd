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


-- Test for or_reduce/and_reduce functions.

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;

entity vhdl_reduce is
    port(inp : in std_logic_vector(4 downto 0);
         and_reduced : out std_logic;
         or_reduced : out std_logic);
end vhdl_reduce;

architecture test of vhdl_reduce is
begin
    process(inp)
    begin
        or_reduced <= or_reduce(inp);
        and_reduced <= and_reduce(inp);
    end process;
end test;
