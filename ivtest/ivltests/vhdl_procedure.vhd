-- Copyright (c) 2014 CERN
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


-- Test for VHDL procedure calls.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_procedure is
  port(run : in std_logic);
end entity vhdl_procedure;

architecture test of vhdl_procedure is

procedure proc(word_i : std_logic_vector(3 downto 0)) is
begin
  report "Procedure executed";
end procedure;

begin
  process(run) begin
    report "before rising_edge";

    if rising_edge(run) then
      proc("0000");
    end if;

    report "after rising_edge";
  end process;
end test;
