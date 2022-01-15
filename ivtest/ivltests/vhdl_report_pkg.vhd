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


-- Report & assert tests.

library ieee;
use ieee.std_logic_1164.all;

package vhdl_report_pkg is
    -- as of the moment of writing vhdlpp does not support procedures
    function test_asserts(a : integer) return integer;
end vhdl_report_pkg;

package body vhdl_report_pkg is

-- Test functions used to output package files
function test_asserts(a : integer) return integer is
begin
    report "procedure 1"
    severity ERROR;

    assert false;

    assert 1 = 0
    report "procedure 2";

    assert 1 = 2
    severity NOTE;

    assert false
    report "procedure 3"
    severity WARNING;

    return 0;
end function;

end vhdl_report_pkg;
