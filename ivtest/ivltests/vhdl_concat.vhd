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


-- Test for expression concatenation in VHDL.

library ieee;
use ieee.std_logic_1164.all;

entity concat is
end concat;

architecture test of concat is
  signal concat1 : std_logic_vector(1 downto 0);
  signal concat2 : std_logic_vector(0 to 4);
begin
  concat1 <= '1' & '0';
  concat2 <= '1' & "10" & concat1;
end test;
