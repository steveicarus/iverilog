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


-- Test for time related expressions.

library ieee;
use ieee.std_logic_1164.all;
use work.time_pkg.all;

entity vhdl_time is
    port(a : out std_logic;
         b : in std_logic;
         tout : out time;
         tin : in time);
end vhdl_time;

architecture test of vhdl_time is
    signal time_sig : time_subtype := 100 ns;
begin
    tout <= 140 ns;

    process(b)
        variable time_var : time;
    begin
        if(rising_edge(b)) then
            time_var := 100 ns;
            time_sig := 500 ns;

            a := '0';
            wait for 50 ns;
            a := '1';
            wait for time_sig;      -- signal
            a := '0';
            wait for time_const;    -- constant
            a := '1';
            wait for time_var;      -- variable
            a := '0';

            wait for (time_sig + time_const + time_var);
            a := '1';

            -- Modify variable & signal values
            time_var := 10 ns;
            wait for time_var;
            a := '0';

            time_sig := 20 ns;
            wait for time_sig;
            a := '1';

            -- Test time read from port
            wait for tin;
            a := '0';
        end if;
    end process;

end test;
