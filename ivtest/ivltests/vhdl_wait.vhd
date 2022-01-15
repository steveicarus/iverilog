-- Copyright (c) 2015 CERN
-- @author Maciej Suminski <maciej.suminski@cern.ch>
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


-- 'wait on' & 'wait until' test

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_wait is
    port(a : in std_logic_vector(1 downto 0);
         b : out std_logic_vector(1 downto 0));
end vhdl_wait;

architecture test of vhdl_wait is
begin
    process begin
        report "final wait test";
        wait;
    end process;

    process begin
        wait on a(0);
        report "wait 1 completed";
        -- acknowledge wait 1
        b(0) <= '1';
    end process;

    process begin
        wait until(a(1) = '1' and a(1)'event);
        report "wait 2 completed";
        -- acknowledge wait 2
        b(1) <= '1';
    end process;
end test;

