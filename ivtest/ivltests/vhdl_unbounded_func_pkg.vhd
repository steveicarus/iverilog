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


-- Basic test for functions that work with unbounded vectors as return
-- and param types.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package vhdl_unbounded_func_pkg is
    function f_manch_encoder (word_i :std_logic_vector) return std_logic_vector;
end vhdl_unbounded_func_pkg;

package body vhdl_unbounded_func_pkg is

function f_manch_encoder (word_i : std_logic_vector) return std_logic_vector is
    variable word_manch_o : std_logic_vector((2*word_i'length) - 1 downto 0);
begin
    for I in word_i'range loop
        word_manch_o (I*2)   := not word_i(I);
        word_manch_o (I*2+1) := word_i(I);
    end loop;

    return word_manch_o;
end function;

end vhdl_unbounded_func_pkg;
