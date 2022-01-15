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


-- Test for accessing constant records & arrays of records in VHDL.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vhdl_const_record is
    port(sel : in integer range 0 to 3;
         hex : out std_logic_vector(7 downto 0);
         aval : out std_logic_vector(7 downto 0));
end entity vhdl_const_record;

architecture test of vhdl_const_record is
  type t_var is (var_presence, var_identif, var_1, var_2, var_3, var_rst, var_4, var_5, var_whatever);
  type t_byte_array is array (natural range <>) of std_logic_vector(7 downto 0);

  type t_var_record is record
    var          : t_var;                           -- 32 bits
    a            : t_byte_array (3 downto 0);       -- 4*8 bits
    hexvalue     : std_logic_vector (7 downto 0);   -- 8 bits
  end record;                                       -- total 72 bits

  type t_var_array is array (natural range <>) of t_var_record;

  constant c_vars_array : t_var_array(0 to 3) := (
     0 => (var      => var_presence,
           hexvalue => x"14",
           a        => (0 => x"aa", 1 => x"ab", 2 => x"ac", 3 => x"ad")),
     1 => (var      => var_identif,
           hexvalue => x"24",
           a        => (0 => x"ba", 1 => x"bb", 2 => x"bc", 3 => x"bd")),
     2 => (var      => var_1,
           hexvalue => x"34",
           a        => (0 => x"ca", 1 => x"cb", 2 => x"cc", 3 => x"cd")),
     3 => (var      => var_2,
           hexvalue => x"56",
           a        => (0 => x"da", 1 => x"db", 2 => x"dc", 3 => x"dd"))
  );

  constant c_record : t_var_record := (
    var      => var_4,
    hexvalue => x"66",
    a        => (0 => x"00", 1 => x"11", 2 => x"22", 3 => x"33")
  );
  signal sig : std_logic_vector(7 downto 0);
  signal sig2 : std_logic_vector(7 downto 0);
begin
  sig <= c_record.hexvalue;

  process(sel)
  begin
      sig2 <= c_record.a(sel);
      hex <= c_vars_array(sel).hexvalue;
      aval <= c_vars_array(sel).a(sel);
  end process;
end architecture test;
