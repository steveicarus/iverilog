--
-- Author: Pawel Szostek (pawel.szostek@cern.ch)
-- Date: 28.07.2011

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dummy is
    port (clk  : in std_logic;
          input : in std_logic_vector(3 downto 0);
          output : out std_logic_vector(15 downto 0)
    );
end;

architecture behaviour of dummy is
begin
    L: process(clk)
        variable one : integer;             -- mix integers and unsigned
        variable a : unsigned (6 downto 0); --
        variable b,c,d : unsigned(6 downto 0);
    begin
        if(clk'event and clk = '1') then
           --do some mess around..
           a(3 downto 0) := unsigned(input);
           a(6 downto 4) := "000";
           one := 1;
           b := a + one; --unsigned plus integer
           b := a + 1; --variable plus constant integer
           c := a + a; --
           c := c - b; --two assignments in a row to the same variable
           d := c + 2;
           output(6 downto 0) <= std_logic_vector(d); --signal assignment
           output(15 downto 7) <= (others => '0');
        end if;
    end process;
end;
