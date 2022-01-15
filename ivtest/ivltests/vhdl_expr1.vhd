-- This VHDL was converted from Verilog using the
-- Icarus Verilog VHDL Code Generator 0.10.0 (devel) (s20090923-519-g6ce96cc)

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity subtract is
  port (
    a : in unsigned(3 downto 0);
    b : in unsigned(3 downto 0);
    out_sig : out unsigned(3 downto 0)
  );
end entity;

architecture test of subtract is
begin
  out_sig <= (a + not b) + 1;
end architecture;
