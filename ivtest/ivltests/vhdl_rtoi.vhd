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


-- Test real to integer conversion

library ieee;
use ieee.numeric_std.all;

entity vhdl_rtoi is
end;

architecture test of vhdl_rtoi is
  signal a, b, c, d  : integer;
begin
  -- test rounding
  a <= integer(2.3); -- should be 2
  b <= integer(3.7); -- should be 4
  c <= integer(4.5); -- should be 5
  d <= integer(8.1 * 2.1); -- ==17.01, should be 17
end test;
