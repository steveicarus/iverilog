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


-- Test for variable initialization.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vhdl_var_init is
  port(init : in std_logic;
       slv  : out std_logic_vector(7 downto 0);
       bool : out boolean;
       i  : out integer
  );
end vhdl_var_init;

architecture test of vhdl_var_init is

begin
  process(init)
    variable var_slv  : std_logic_vector(7 downto 0) := "01000010";
    variable var_bool : boolean := false;
    variable var_int  : integer := 42;
  begin
    if rising_edge(init) then
      slv  <= var_slv;
      bool <= var_bool;
      i    <= var_int;
    end if;
  end process;
end test;
