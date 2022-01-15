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


-- Report & assert tests.

library ieee;
use ieee.std_logic_1164.all;
use work.vhdl_report_pkg.all;

entity vhdl_report is
    port (start_test : in std_logic);
end vhdl_report;

architecture test of vhdl_report is
begin
    process(start_test)
    begin
        if(start_test = '1') then
            -- Report without severity specified, by default it is NOTE
            report "normal report";

            -- Report with severity specified
            -- should continue execution when severity != FAILURE
            report "report with severity"
            severity ERROR;

            -- Assert, no report, no severity specified, by default it is ERROR
            assert false;

            -- Assert with report, no severity specified, by default it is ERROR
            assert 1 = 0
            report "assert with report";

            -- Assert without report, severity specified
            -- should continue execution when severity != FAILURE
            assert 1 = 2
            severity NOTE;

            -- Assert with report and severity specified
            assert false
            report "assert with report & severity"
            severity FAILURE;   -- FAILURE causes program to stop

            report "this should never be shown";
        end if;
    end process;
end architecture test;
