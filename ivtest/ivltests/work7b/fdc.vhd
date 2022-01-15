-- a D-type flip-flop with synchronous reset

library ieee;
use ieee.std_logic_1164.all;


entity fdc is
port (clk: in std_logic;
	  reset: in std_logic;
      d: in std_logic;
      q: out std_logic
);
end fdc;

architecture fdc_rtl of fdc is
begin

i_finish: process (clk)
begin
   if (clk'event and clk = '1') then
     if (reset = '1') then
	    q <= '0';
	  else
        q <= d;
	 end if;
   end if;
end process;

end fdc_rtl;
