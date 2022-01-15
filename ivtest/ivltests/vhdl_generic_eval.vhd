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


-- Test for generics evaluation.

library ieee;
use ieee.std_logic_1164.all;

entity eval_generic is
  generic(
        msb : integer range 1 to 7 := 7;
        bit_select : integer range 0 to 7 := 3
  );
  port(
        in_word : in std_logic_vector(msb downto 0);
        out_bit : out std_logic
  );
end entity eval_generic;

architecture test of eval_generic is
begin
    out_bit <= in_word(bit_select);
end architecture test;


library ieee;
use ieee.std_logic_1164.all;

entity test_eval_generic is
  port(
        in_word : in std_logic_vector(7 downto 0);
        out_bit_def, out_bit_ovr : out std_logic
  );
end entity test_eval_generic;

architecture test of test_eval_generic is
    constant const_int : integer := 7;

    component eval_generic is
    generic(
          msb : integer range 1 to 7;
          bit_select : integer range 0 to 7
    );
    port(
            in_word : in std_logic_vector(msb downto 0);
            out_bit : out std_logic
    );
    end component eval_generic;
begin
    override_test_unit: eval_generic
    generic map(bit_select => 2,
                msb => const_int)
    port map(
                in_word => (others => '1'),
                out_bit => out_bit_ovr
    );

    default_test_unit: eval_generic
    port map(
                in_word => in_word,
                out_bit => out_bit_def
    );
end architecture test;
