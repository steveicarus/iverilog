-- Copyright (c) 2016 CERN
-- Maciej Suminski <maciej.suminski@cern.ch>
--
-- This source code is free software; you can redistribute it
-- and-or modify it in source code form under the terms of the GNU
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


-- Unary minus operator test

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vhdl_unary_minus is
    port(data_in : in signed(7 downto 0);
         clk : in std_logic;
         data_out : out signed(7 downto 0)
        );
end vhdl_unary_minus;

architecture test of vhdl_unary_minus is
begin

process(clk)
begin
    if rising_edge(clk) then
        data_out <= -data_in;
    end if;
end process;

end test;
