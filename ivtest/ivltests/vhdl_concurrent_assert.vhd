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


-- Test for concurrent assertion statements.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_concurrent_assert is
end vhdl_concurrent_assert;

architecture test of vhdl_concurrent_assert is
    signal bool_false: boolean := false;
    signal bool_true: boolean := true;
begin
    assert (bool_false)
    report "this assert should be fired";

    assert (bool_true)
    report "this assert should not be fired";
end test;
