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


-- Basic test for 'real' floating-type support in VHDL.

library ieee;

entity vhdl_real is
end;

architecture test of vhdl_real is
  constant c      : real := 1111.222;
  constant d      : real := 23.8;
  constant e      : real := c + d;

  signal a        : real := 1.2;
  signal b        : real := 32.123_23;
  signal pi       : real := 3.14159265;
  signal exp      : real := 2.334E+2;
  signal no_init  : real;
begin
  no_init <= a + b;
end test;
