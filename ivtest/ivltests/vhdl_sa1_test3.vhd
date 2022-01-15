library ieee;
use ieee.std_logic_1164.all;

package work6 is

  -- D-type flip flop
  component fdc is
    port (clk: in std_logic;
	      reset: in std_logic;
          d: in std_logic;
          q: out std_logic);
  end component;

 component TimeBase is
  port(
    CLOCK : in std_logic;          -- input clock of 20MHz
    TICK : out std_logic;          -- out 1 sec timebase signal
    RESET  : in std_logic;         -- master reset signal (active high)
    ENABLE : in std_logic;
    COUNT_VALUE: out std_logic_vector (24 downto 0)
  );
end component;

end package work6;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-- The operation is:
-- 1) An internal counter (of 25 bits) is initilaised to zero after a reset is received.
-- 2) An enable allows an internal running counter to count clock pulses
-- 3) A tick signal output is generated when a count of 20000000 pulses has been accumulated


entity TimeBase is
  port(
    CLOCK : in std_logic;          -- input clock of 20MHz
    TICK : out std_logic;          -- out 1 sec timebase signal
    RESET  : in std_logic;         -- master reset signal (active high)
    ENABLE : in std_logic;
    COUNT_VALUE: out std_logic_vector (24 downto 0)
  );
end TimeBase;

architecture TimeBase_rtl of TimeBase is

  constant DIVIDER_VALUE : std_logic_vector := x"7cf";  -- 20000000 count value, 1 second
  signal RunningCounter  : std_logic_vector(24 downto 0);   -- this is the 25 bit free running counter to allow a big count
begin

  RunningCounterProcess : process (CLOCK)
  begin
    if ( CLOCK'event and CLOCK = '1') then
       if (RESET = '1') then
         RunningCounter <= '0' & x"000000";
       elsif ( ENABLE = '1') then
         RunningCounter <= RunningCounter + 1;
       end if;
    else
         RunningCounter <= RunningCounter;
    end if;
  end process;

 TICK <= '1' when (RunningCounter = DIVIDER_VALUE) else '0';

COUNT_VALUE <= RunningCounter;

end TimeBase_rtl;

library ieee;
use ieee.std_logic_1164.all;
use work.work6.all;

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


end bigcount_rtl;-- a D-type flip-flop with synchronous reset

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
