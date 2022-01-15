-- Copyright (c) 2015-2016 CERN
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


-- Test reading files using std.textio library.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;

entity vhdl_textio_read is
    port(
        clk, active : in std_logic;
        line_counter : out integer;
        ok : out std_logic
    );
end vhdl_textio_read;

architecture test of vhdl_textio_read is
begin
    read_data: process(clk, active)
    file data_file              : text;
    variable data_line          : line;

    variable data_string        : string(6 downto 1);
    variable data_int, data_hex : integer;
    variable data_bool          : boolean;
    variable data_real          : real;
    variable data_time          : time;
    variable data_logic         : std_logic_vector(5 downto 0);

    begin
        if rising_edge(active) then
            file_open(data_file, "vhdl_textio.tmp", read_mode);
            line_counter := 0;
        elsif falling_edge(active) then
            file_close(data_file);
        end if;

        if rising_edge(clk) and active = '1' then
            readline(data_file, data_line);
            line_counter := line_counter + 1;

            case line_counter is
                -- Test reading different variable types
                when 1 => read(data_line, data_int);
                when 2 => read(data_line, data_bool);
                when 3 => read(data_line, data_time);
                when 4 => hread(data_line, data_hex);
                when 5 => read(data_line, data_real);
                when 6 => read(data_line, data_string);
                when 7 =>
                    read(data_line, data_logic);

                    -- Verify the read data
                    if data_int = 123
                        and data_bool = true
                        and data_time = 100 s
                        and data_hex = x"f3"
                        and data_real = 12.21
                        and data_string = "string"
                        and data_logic = "1100XZ" then
                        ok <= '1';
                    end if;
            end case;
        end if;
    end process;
end test;
