--
-- Author: Pawel Szostek (pawel.szostek@cern.ch)
-- Date: 27.07.2011

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity match_bits is
    port (a,b: in std_logic_vector(7 downto 0);
            matches : out std_logic_vector(7 downto 0)
    );
end;

architecture behaviour of match_bits is
begin
    process(a, b) begin
        for i in 7 downto 0 loop
            matches(i) <= not (a(i) xor b(i));
        end loop;
    end process;
end;
