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


-- Test writing files using std.textio library.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;

entity vhdl_textio_write is
    port(
        wr : in std_logic
    );
end vhdl_textio_write;

architecture test of vhdl_textio_write is

begin
    write_data: process(wr)
    file data_file          : text open write_mode is "vhdl_textio.tmp";
    variable data_line      : line;

    variable data_string    : string(6 downto 1);
    variable data_int, data_hex : integer;
    variable data_bool      : boolean;
    variable data_real      : real;
    variable data_time      : time;
    variable data_vector    : std_logic_vector(5 downto 0);

    begin
        data_string := "string";
        data_int := 123;
        data_hex := X"F3";
        data_bool := true;
        data_real := 12.21;
        data_time := 100 s;
        data_vector := "1100XZ";

        -- Test writing different variable types
        write(data_line, data_int);
        writeline(data_file, data_line);
        write(data_line, data_bool);
        writeline(data_file, data_line);
        write(data_line, data_time);
        writeline(data_file, data_line);

        hwrite(data_line, data_hex);
        writeline(data_file, data_line);
        write(data_line, data_real);
        writeline(data_file, data_line);
        write(data_line, data_string);
        writeline(data_file, data_line);
        write(data_line, data_vector);
        writeline(data_file, data_line);
    end process;
end test;
