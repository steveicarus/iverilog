library ieee;
use ieee.std_logic_1164.all;

-- This is a simple test of the initialization assignment for
-- signals. We also let a generic into the test.

entity test is

  generic (width : integer := 4);

  port (clk  : in std_logic;
        src0, src1 : in  std_logic_vector (width-1 downto 0);
        dst  : out std_logic_vector (width-1 downto 0));

end test;

library ieee;
use ieee.std_logic_1164.all;

entity reg_xor is
  port (clk : in std_logic;
        src0, src1 : in  std_logic;
        dst : out std_logic);
end reg_xor;

architecture operation of test is
  component reg_xor
    port (clk        : in  std_logic;
          src0, src1 : in  std_logic;
          dst        : out std_logic);
  end component;
begin
  vec: for idx in width-1 downto 0 generate
    slice: reg_xor port map (clk  => clk,
                             src0 => src0(idx),
                             src1 => src1(idx),
                             dst  => dst(idx));
  end generate vec;
end operation;

architecture operation of reg_xor is
  signal tmp : std_logic;
begin
  tmp <= src0 xor src1;
  step: process (clk)
  begin  -- process step
    if  clk'event and clk = '1' then  -- rising clock edge
      dst <= tmp;
    end if;
  end process step;
end operation;
