--
-- Author: Pawel Szostek (pawel.szostek@cern.ch)
-- Date: 27.07.2011

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dummy is
    port (o1: out std_logic_vector(7 downto 0); -- intentionally messed indices
          i1: in std_logic_vector(0 to 7)       --
    );
end;

architecture behaviour of dummy is
begin
    o1 <= i1;
end;
