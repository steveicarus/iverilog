-- Copyright (c) 2015-2016 CERN
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


-- Test for shift operators (logical and arithmetic)

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_bit.all;

entity shifter is
    port(input : in signed(7 downto 0);
         out_srl, out_sll, out_sra, out_sla : out signed(7 downto 0);
         out_shl_u, out_shr_u : out unsigned(7 downto 0);
         out_shl_s, out_shr_s : out signed(7 downto 0)
    );
end entity shifter;

architecture test of shifter is
begin
    process(input)
        -- test the unsigned variant of shift_left/right() functions
        variable unsigned_input : unsigned(7 downto 0);
    begin
        unsigned_input := input;
        out_srl <= input srl 1;
        out_sll <= input sll 1;
        out_sra <= input sra 1;
        out_sla <= input sla 1;
        out_shl_s <= shift_left(input, 2);
        out_shr_s <= shift_right(input, 2);
        out_shl_u <= shift_left(unsigned_input, 2);
        out_shr_u <= shift_right(unsigned_input, 2);
    end process;
end architecture test;
