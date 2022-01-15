--
-- Author: Pawel Szostek (pawel.szostek@cern.ch)
-- Date: 27.07.2011

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity count_ones is
    port ( vec: in std_logic_vector(15 downto 0);
            count : out unsigned(4 downto 0));
end;

architecture behaviour of count_ones is
begin
    process(vec)
        variable result : unsigned(4 downto 0);
    begin
        result := to_unsigned(0, result'length);
        for i in 15 downto 0 loop
            if vec(i) = '1' then
                    result := result +1;
            end if;
        end loop;
        count <= result;
    end process;
end;
