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


-- Test for constant arrays access

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.constant_array_pkg.all;

entity constant_array is
    port (index  : in  std_logic_vector(2 downto 0);
          output : out std_logic_vector(7 downto 0));
end entity constant_array;

architecture test of constant_array is
  type logic_array is array (integer range <>) of std_logic_vector(0 to 3);
  constant test_array : logic_array(0 to 5) :=
    (0      => "0010",
     1      => "1000",
     2      => "0100",
     3      => "0110",
     4      => "0101",
     5      => "1100");

  -- Check if constant vectors are not broken with the changes
  constant vector : std_logic_vector(5 downto 0) := "110011";

  signal test_a : unsigned(7 downto 0);
  signal test_b : std_logic_vector(3 downto 0);
  signal test_c : std_logic_vector(2 downto 0);
  signal test_d : std_logic;
begin
  test_a <= const_array(3);
  test_b <= test_array(2);
  test_c <= vector(4 downto 2);
  test_d <= vector(5);

  process (index)
  begin
    output <= const_array(to_integer(unsigned(index)));
  end process;
end architecture test;
