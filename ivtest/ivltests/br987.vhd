-- Reduced test case, bug originally found in 4DSP's fmc110_ads5400_ctrl.vhd
entity bug5 is
port (
  clk       : in  std_logic;
  trig      : in  std_logic;
  data_o    : out std_logic
);
end bug5;

architecture bug5_syn of bug5 is
begin

dummy:process(clk)
begin
  if (trig = '1') then
    --data_o <= '1';  -- uncomment this and everythings's OK
  end if;
end process dummy;

end bug5_syn;
