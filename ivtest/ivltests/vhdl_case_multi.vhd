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

-- Test for multiple choices in case alternative statements.

library ieee;
use ieee.std_logic_1164.all;

entity vhdl_case_multi is
  port ( inp: in std_logic_vector (0 to 2);
         parity: out std_logic );
end vhdl_case_multi;

architecture vhdl_case_multi_rtl of vhdl_case_multi is
begin

  process (inp)
  begin
    case inp is
        when "000"|"011"|"101"|"110" => parity <= "0";
        when "001"|"010"|"100"|"111" => parity <= "1";
        when others  => parity <= "Z";
    end case;
  end process;

end vhdl_case_multi_rtl;
