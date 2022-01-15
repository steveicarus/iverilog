-- Copyright (c) 2016 CERN
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


-- Test for 'image attribute in VHDL.

library ieee;
use ieee.std_logic_1164.all;

entity image_attr_entity is
    port (start_test : in std_logic);
end image_attr_entity;

architecture test of image_attr_entity is
begin
    process(start_test)
        variable var_int : integer := 10;
        variable var_real : real := 12.34;
        variable var_char : character := 'o';
        variable var_time : time := 10 ns;
    begin
        if(start_test = '1') then
            report "integer'image test: " & integer'image(var_int);
            report "real'image test: " & real'image(var_real);
            report "character'image test: " & character'image(var_char);
            report "time'image test: " & time'image(var_time);
        end if;
    end process;
end test;
