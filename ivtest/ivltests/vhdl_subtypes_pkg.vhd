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


-- Test for subtype definitions.

library ieee;
use ieee.numeric_bit.all;

package vhdl_subtypes_pkg is
    constant type_range : integer := 10;

    subtype int_type_const is integer range 0 to type_range-1;
    subtype int_type is integer range 0 to 7;
    subtype int_type_downto is integer range 8 downto 1;
    subtype time_type is time range 0 fs to 1 ms;
    subtype uns_type_const is unsigned(7 downto 0);
end vhdl_subtypes_pkg;

package body vhdl_subtypes_pkg is
end vhdl_subtypes_pkg;

