------------------------------------------------------------------------------
-- Author: Oswaldo Cadenas
-- Date: September 27 2011
--
-- Summary: This system runs an internal counter 0,1,2, ..., 7, 0, 1,
--          and also accepts an enable signal
--          it generates an output y (4 bits) as:
--          if (e = 0) y = 0000
--          if (e = 1) then
--              y = 1000 when counter is 0
--              y = 0100 when counter is 1
--              y = 0010 when counter is 2
--              y = 0001 when counter is 3
--              y = 1111 other count
-- internaly the design uses some enumartion arguments for decoding and encoding
---------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity enumsystem is
  port (clk, reset: in std_logic;
        en: in std_logic;                   -- enable
        y: out std_logic_vector (0 to 3) ); -- decoded output
end enumsystem;

architecture enumsystem_rtl of enumsystem is

  type States is (zero, one, two, three, other);
  signal mystate: States;

  signal Counter: std_logic_vector (2 downto 0);

  begin

  SmallCounter : process (clk, reset)
  begin
    if ( clk'event and clk = '1') then
       if (reset = '1') then
         Counter <= (others => '0');
       else
         Counter <= Counter + 1;
       end if;
  end if;
  end process;


  encoding_process: process (Counter)
      begin
          case Counter is
             when "000"   => mystate <= zero;
             when "001"   => mystate <= one;
             when "010"   => mystate <= two;
             when "011"   => mystate <= three;
             when others  => mystate <= other;
          end case;
  end process;


  decoding_process:  process (mystate, en)
      begin
        if (en = '1') then
          case mystate is
             when zero  => y <= "1000";
             when one   => y <= "0100";
             when two   => y <= "0010";
             when three => y <= "0001";
             when others => y <= "1111";
          end case;
        else
           y <= "0000";
        end if;
  end process;

end enumsystem_rtl;
