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

-- Tests for various subprogram features.

library ieee;
use ieee.std_logic_1164.all;
-- tests subprograms imported from an external library
use work.subprogram_pkg.all;

entity subprogram is
end;

architecture behaviour of subprogram is
  function negate(input_word : std_logic_vector(7 downto 0))
  -- tests using undefined size std_logic_vector as a return type
  return std_logic_vector is
    -- tests variable declaration in subprograoms
    variable output_word : std_logic_vector(7 downto 0);
  begin
    for i in 7 downto 0 loop
      -- tests distuingishing between vector and function call basing on the
      -- function parameter
      output_word(i) := not input_word(i);
    end loop;

    return output_word;
  end function;

  signal negated : std_logic_vector(7 downto 0);
  signal reversed : std_logic_vector(7 downto 0);
begin
  -- parameter type is determined by checking the parameter type in function declaration
  negated <= negate("11000111");
  reversed <= reverse("10000111");
end;
