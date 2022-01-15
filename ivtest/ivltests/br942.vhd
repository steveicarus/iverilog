library ieee;
use ieee.std_logic_1164.all;

entity e is
  port (
    clk : in  std_logic;
    rst : in  std_logic;
    q   : out std_logic);
end e;

architecture a of e is

  signal r : std_logic;

  function invert (
    i : std_logic)
    return std_logic is
  begin
    return not i;
  end invert;
begin

   q <= r;

   process(clk)
   begin
     if rising_edge(clk) then
       if rst = '1' then
         r <= '0';
       else
         r <= invert(r);
       end if;
     end if;
   end process;
end a;
