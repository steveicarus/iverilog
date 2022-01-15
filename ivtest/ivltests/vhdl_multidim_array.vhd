-- Copyright (c) 2016 CERN
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


-- Test multidimensional arrays.

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhdl_multidim_array is
end vhdl_multidim_array;

architecture archi of vhdl_multidim_array is
    constant array_size : integer := 16;
    subtype one_dim is unsigned(array_size - 1 downto 0);
    type multi_dim is array (0 to 1) of one_dim;

begin
    process
        variable arr : multi_dim;
    begin
        -- fill the array with test data
        for i in array_size - 1 downto 0
        loop
            arr(0)(i) := 2 ** i;
            arr(1)(i) := not arr(0)(i);
        end loop;
        wait;
    end process;
end archi;
