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


-- Basic test for the unbounded arrays in VHDL.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_unbounded_array is
end vhdl_unbounded_array;

architecture test of vhdl_unbounded_array is
  -- This can be translated as an unpacked array in SystemVerilog
  type unb_logic is array (integer range <>) of std_logic;
  -- These have to be packed arrays
  type unb_integer is array (natural range <>) of integer;
  type unb_real is array (integer range <>) of real;

  signal sig_logic : unb_logic(7 downto 0);
  signal sig_integer : unb_integer(3 downto 0);
  signal sig_real : unb_real(0 to 3);
begin
  sig_logic <= "01010101";
  sig_integer(2) <= 1;
  sig_real(1) <= 2.5;
end architecture test;
