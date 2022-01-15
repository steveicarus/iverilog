-- Reduced test case, bug originally found in 4DSP's fmc110_ctrl.vhd
library ieee;
  use ieee.std_logic_1164.all;

entity bug3 is
port (
  sel             : in  std_logic_vector(3 downto 0);
  value           : in  std_logic;
  spi_sdo         : out std_logic
);
end bug3;

architecture bug3_syn of bug3 is
begin

-- no problem if the "not" is taken out
spi_sdo <=  not or_reduce(sel);

end bug3_syn;
