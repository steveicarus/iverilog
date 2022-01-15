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


-- Test for labeled assignment statements

library ieee;
use ieee.std_logic_1164.all;

entity labeled_assign is
    port (input : in std_logic_vector(7 downto 0);
          output : out std_Logic_vector(7 downto 0));
end entity;

architecture test of labeled_assign is
    signal test_rx : std_logic_vector (7 downto 0);
begin
    first_label: test_rx <= x"aa";

    process(input)
        variable tmp : std_logic_vector(7 downto 0);
    begin
        second_label: tmp := input;
        third_label: output <= tmp xor x"cc";
    end process;

end architecture;
