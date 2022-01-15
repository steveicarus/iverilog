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


-- Test for to_integer() function.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity to_int is
    port (
        unsign : in unsigned(7 downto 0);
        sign : in signed(7 downto 0)
    );
end to_int;

architecture test of to_int is
  signal s_natural : natural;
  signal s_integer : integer;
begin

process (unsign)
begin
    s_natural <= (unsign);
end process;

process (sign)
begin
    s_integer <= (sign);
end process;

end test;
