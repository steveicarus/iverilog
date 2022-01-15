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


-- Test file_open() function.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;

entity vhdl_file_open is
    port(active : in std_logic;
         ok : out std_logic);
end vhdl_file_open;

architecture test of vhdl_file_open is
begin
    process(active)
        file ok_file, bad_file : text;
        variable ok_status, bad_status : FILE_OPEN_STATUS;
    begin
        if rising_edge(active) then
            file_open(ok_status, ok_file, "ivltests/vhdl_file_open.vhd", read_mode);
            file_open(bad_status, bad_file, "not_existing_file", read_mode);

            if ok_status = OPEN_OK and bad_status = NAME_ERROR then
                ok := '1';
            else
                ok := '0';
            end if;
            file_close(ok_file);
        end if;
    end process;
end test;
