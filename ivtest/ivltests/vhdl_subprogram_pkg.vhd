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

-- tests functions defined in packages
package subprogram_pkg is
  function reverse(input_word : std_logic_vector(7 downto 0))
  return std_logic_vector;
end subprogram_pkg;

package body subprogram_pkg is
  function reverse(input_word : std_logic_vector(7 downto 0))
  return std_logic_vector is
    variable output_word : std_logic_vector(7 downto 0);
  begin
    for i in 7 downto 0 loop
      output_word(i) := input_word(7 - i);
    end loop;

    return output_word;
  end function;
end subprogram_pkg;
