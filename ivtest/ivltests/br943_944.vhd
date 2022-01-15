library ieee;
use ieee.std_logic_1164.all;

entity e is
  port (
    clk : in  std_logic;
    rst : in  std_logic;
    q   : out std_logic);
end e;

architecture a of e is

  type t is (one, zero);
  signal r : t;

begin

   q <= '1' when r = one else '0';

   process(clk)
   begin
     if rising_edge(clk) then
       if rst = '1' then
         r <= zero;
       else
         case r is
           when zero   => r <= one;
           when others => r <= zero;
         end case;
       end if;
     end if;
   end process;
end a;
