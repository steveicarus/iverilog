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


-- Basic test for functions that work with unbounded vectors as return
-- and param types.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.vhdl_unbounded_func_pkg.all;

package included_pkg is
    function negator(word_i : std_logic_vector) return std_logic_vector;
end included_pkg;

package body included_pkg is
function negator(word_i : std_logic_vector) return std_logic_vector is
  variable word_o : std_logic_vector (word_i'left downto word_i'right);
begin
    for I in word_i'range loop
        word_o (I) := not word_i(I);
    end loop;

    return word_o;
end function;
end included_pkg;

entity vhdl_unbounded_func is
end vhdl_unbounded_func;

architecture test of vhdl_unbounded_func is
    signal test_out1 : std_logic_vector(9 downto 0);
    signal test_out2 : std_logic_vector(5 downto 0);

    signal neg_test_out1 : std_logic_vector(9 downto 0);
    signal neg_test_out2 : std_logic_vector(5 downto 0);

begin
    test_out1 <= f_manch_encoder(B"11101");
    test_out2 <= f_manch_encoder(B"001");
    neg_test_out1 <= negator(test_out1);
    neg_test_out2 <= negator(test_out2);
end test;

