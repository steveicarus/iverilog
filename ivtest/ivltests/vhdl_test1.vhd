--
-- Author: Pawel Szostek (pawel.szostek@cern.ch)
-- Date: 28.07.2011

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity mask is
    port (input : in std_logic_vector(15 downto 0);
          output : out std_logic_vector(15 downto 0)
    );
end;

architecture behaviour of mask is
begin
    L: process(input)
        variable tmp : std_logic_vector(15 downto 0);
    begin
        output <= tmp; --this shouln't really change anything
        tmp := input;
        tmp := tmp and "1010101010101010";
        output <= tmp;
    end process;
end;
