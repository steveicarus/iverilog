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


-- Test for 'range, 'left and 'right attributes in VHDL subprograms.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package range_func_pkg is
  function negator (word_i : std_logic_vector(3 downto 0)) return std_logic_vector;
  function reverse (word_i : std_logic_vector(3 downto 0)) return std_logic_vector;
end range_func_pkg;

package body range_func_pkg is

function negator (word_i : std_logic_vector(3 downto 0)) return std_logic_vector is
    variable neg : std_logic_vector(word_i'left downto word_i'right);
begin
    for I in word_i'range loop
      neg (I) := not word_i(I);
    end loop;

    return neg;
end function;

function reverse (word_i : std_logic_vector(3 downto 0)) return std_logic_vector is
    variable rev : std_logic_vector(3 downto 0);
begin
    for I in word_i'right to word_i'left loop
      rev (rev'left - I) := word_i(I);
    end loop;

    return rev;
end function;

end range_func_pkg;
