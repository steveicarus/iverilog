library ieee;
use ieee.std_logic_1164.all;

entity dec2to4 is
  port (sel: in std_logic_vector (1 downto 0);
        en: in std_logic;
        y: out std_logic_vector (0 to 3) );
end dec2to4;

architecture dec2to4_rtl of dec2to4 is
  begin
    process (sel, en)
      begin
        if (en = '1') then
          case sel is
             when "00" => y <= "1000";
             when "01" => y <= "0100";
             when "10" => y <= "0010";
             when "11" => y <= "0001";
             when others => y <= "0000";
          end case;
        else
           y <= "0000";
        end if;
  end process;
end dec2to4_rtl;
