library ieee;
use ieee.std_logic_1164.all;

entity mux2to1 is
  port (i0, i1, s: in std_logic;
        y: out std_logic);
end mux2to1;

architecture mux2to1_rtl of mux2to1 is
begin
    process (i0, i1, s)
      begin
        case (s) is
          when '0' => y <= i0;
          when others => y <= i1;
        end case;
      end process;
end mux2to1_rtl;
