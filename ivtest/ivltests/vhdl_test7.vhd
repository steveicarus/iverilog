--
-- Author: Pawel Szostek (pawel.szostek@cern.ch)
-- Date: 28.07.2011

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dummy is
    port (
        input : in std_logic_vector(7 downto 0);
        output : out std_logic_vector(7 downto 0)
    );
end;

architecture behaviour of dummy is
begin
    L: process(input)
        variable tmp : std_logic_vector(7 downto 0);
    begin
       tmp := input;                            -- use multiple assignments to the same variable
       tmp := (7 => input(7), others => '1');   -- inluding slices in a process
       output <= tmp;
    end process;
end;
