library ieee;
use ieee.std_logic_1164.all;
use work.work7.all;

entity bigcount is
  port (clk, reset: in std_logic;
        count: out std_logic_vector (24 downto 0)
        );
end entity bigcount;

architecture bigcount_rtl of bigcount is
signal d, t, q, myreset: std_logic;
begin

d <= t xor q;

myreset <= reset or t;

f1: fdc port map (clk => clk, reset => reset, d => d,   q => q);
tb: timebase port map (CLOCK => clk, RESET => myreset, ENABLE => '1', TICK => t, COUNT_VALUE => open );

counting: timebase port map (CLOCK => clk, RESET => reset, ENABLE => q, TICK => open, COUNT_VALUE => count );


end bigcount_rtl;
