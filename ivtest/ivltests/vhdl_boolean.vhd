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


-- boolean values test.

library ieee;

entity vhdl_boolean is
end vhdl_boolean;

architecture test of vhdl_boolean is
    signal true_val, false_val, and1, and2, and3, or1, or2, or3, not1, not2 : boolean;
begin
    true_val <= true;
    false_val <= false;

    and1 <= true and true;
    and2 <= true and false;
    and3 <= false and false;

    or1 <= true or true;
    or2 <= true or false;
    or3 <= false or false;

    not1 <= not false;
    not2 <= not true;
end architecture test;
