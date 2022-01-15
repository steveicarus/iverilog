library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-- The operation is:
-- 1) An internal counter is initilaised to zero after a reset is received.
-- 2) An enable allows an internal running counter to count clock pulses
-- 3) A tick signal output is generated when a the number of pulses accumulated
--    are equal to a specified parameter


entity TimeBase is
  generic (N: in Natural := 12; VALUE: Natural := 1999);
  port(
    CLOCK : in std_logic;          -- input clock of 20MHz
    TICK : out std_logic;          -- out 1 sec timebase signal
    RESET  : in std_logic;         -- master reset signal (active high)
    ENABLE : in std_logic;
    COUNT_VALUE: out std_logic_vector (N-1 downto 0)
  );
end TimeBase;

architecture TimeBase_rtl of TimeBase is

  signal RunningCounter  : std_logic_vector(N-1 downto 0);   -- this is the N bit free running counter to allow a big count
begin

  RunningCounterProcess : process (CLOCK)
  begin
    if ( CLOCK'event and CLOCK = '1') then
       if (RESET = '1') then
         RunningCounter <= (others => '0');
       elsif ( ENABLE = '1') then
         RunningCounter <= RunningCounter + 1;
       end if;
    else
         RunningCounter <= RunningCounter;
    end if;
  end process;

 TICK <= '1' when (RunningCounter = VALUE) else '0';

COUNT_VALUE <= RunningCounter;

end TimeBase_rtl;
