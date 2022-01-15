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


-- Test string escaping in VHDL.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_string is
    port (start : in std_logic);
end entity vhdl_string;

architecture test of vhdl_string is
begin
  process (start)
      variable test_str : string(1 to 4) := "VHDL";
  begin
    report "";
    report """";
    report "test";
    report test_str;

    report ("brackets test");
    report ((("multiple brackets test")));
    report """quotation "" marks "" test""";
  end process;
end architecture test;
