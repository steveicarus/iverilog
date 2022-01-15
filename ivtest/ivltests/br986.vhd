-- Reduced test case, bug originally found in 4DSP's fmc110_ctrl.vhd
library ieee;
  use ieee.std_logic_1164.all;

entity bug3 is
port (
  clk1_i        : in  std_logic;
  clk1_ib       : in  std_logic;
  clk1_o        : out std_logic
);
end bug3;

architecture bug3_syn of bug3 is

component IBUFDS generic (
    DIFF_TERM : boolean := FALSE
); port(
    O : out std_logic;
    I : in std_logic;
    IB : in std_logic
); end component;

begin

ibufds1 : ibufds
 generic map (
   DIFF_TERM => TRUE   -- change to "1" and vhdlpp is happy
 )
port map (
  i  => clk1_i,
  ib => clk1_ib,
  o  => clk1_o
);

end bug3_syn;

entity ibufds is
generic (
  DIFF_TERM : boolean := FALSE
);
port (
  i     : in  std_logic;
  ib    : in  std_logic;
  o     : out std_logic
);
end ibufds;

architecture ibufds_sim of ibufds is

begin
o <= i;

end ibufds_sim;
