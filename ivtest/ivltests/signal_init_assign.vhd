library ieee;
use ieee.std_logic_1164.all;

-- This is a simple test of the initialization assignment for
-- signals. We also let a generic into the test.

entity test is
  generic (PARM : std_logic := '0');
  port (clk : in std_logic;
        src : in  std_logic;
        dst : out std_logic);
end test;

architecture operation of test is
  signal tmp : std_logic := PARM;
begin
  step: process (clk)
  begin  -- process step
    if  clk'event and clk = '1' then  -- rising clock edge
      dst <= tmp xor src;
    end if;
  end process step;
end operation;
