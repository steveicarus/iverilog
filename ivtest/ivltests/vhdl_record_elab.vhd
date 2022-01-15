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


-- Tests initialization of records with aggregate expressions.
-- (based on the vhdl_struct_array test)

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_record_elab is
  port (
    i_low0: in std_logic_vector (3 downto 0);
    i_high0: in std_logic_vector (3 downto 0);
    i_low1: in std_logic_vector (3 downto 0);
    i_high1: in std_logic_vector (3 downto 0);
    o_low0: out std_logic_vector (3 downto 0);
    o_high0: out std_logic_vector (3 downto 0);
    o_low1: out std_logic_vector (3 downto 0);
    o_high1: out std_logic_vector (3 downto 0)
  );
end vhdl_record_elab;

architecture test of vhdl_record_elab is

type word is record
  high: std_logic_vector (3 downto 0);
  low: std_logic_vector (3 downto 0);
end record;

type dword is array (1 downto 0) of word;

signal my_dword : dword;
signal dword_a  : dword;

begin
  -- inputs
  my_dword(0) <= (low => i_low0, high => i_high0);
  -- test if you can assign values in any order
  my_dword(1) <= (high => i_high1, low => i_low1);

  dword_a <= (0 => (low => "0110", high => "1001"),
              1 => (high => "1100", low => "0011"));

  -- outputs
  o_low0 <= my_dword(0).low;
  o_high0 <= my_dword(0).high;
  o_low1 <= my_dword(1).low;
  o_high1 <= my_dword(1).high;
end test;
