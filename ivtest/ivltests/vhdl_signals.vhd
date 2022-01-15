-- This VHDL was converted from Verilog using the
-- Icarus Verilog VHDL Code Generator 0.10.0 (devel) (s20090923-656-gce5c263)

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Generated from Verilog module test (foo.v:3)
entity test is
  port (
    i : in unsigned(3 downto 0);
    o : out std_logic
  );
end entity;

-- Generated from Verilog module test (foo.v:3)
architecture from_verilog of test is
  signal tmp_s1 : std_logic;  -- Temporary created at foo.v:6
  signal tmp_s11 : std_logic;  -- Temporary created at foo.v:6
  signal tmp_s3 : std_logic;  -- Temporary created at foo.v:6
  signal tmp_s4 : std_logic;  -- Temporary created at foo.v:6
  signal tmp_s7 : std_logic;  -- Temporary created at foo.v:6
  signal tmp_s8 : std_logic;  -- Temporary created at foo.v:6
begin
  tmp_s4 <= tmp_s1 and tmp_s3;
  tmp_s8 <= tmp_s4 and tmp_s7;
  o <= tmp_s8 and tmp_s11;
  tmp_s1 <= i(3);
  tmp_s3 <= i(2);
  tmp_s7 <= i(1);
  tmp_s11 <= i(0);
end architecture;
