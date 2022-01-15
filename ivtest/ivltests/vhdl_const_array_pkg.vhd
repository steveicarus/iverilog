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


-- Test for constant arrays access

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package constant_array_pkg is
  type t_unsigned_array is array (natural range <>) of unsigned(7 downto 0);
  constant const_array : t_unsigned_array(7 downto 0) :=
    (0      => "00000010",
     1      => "00001000",
     2      => "00010000",
     3      => "00100000",
     4      => "01000000",
     5      => "01111100",
     others => "00000010");
end package constant_array_pkg;
