library ieee;
use ieee.std_logic_1164.all;

package work7 is

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

end package work7;
