library IEEE;
use IEEE.std_logic_1164.all;


entity or_gate is
  port (
       a_i : in std_logic;    -- inputs
       b_i : in std_logic;
       c_o : out std_logic    -- output
       );
end entity or_gate;

architecture rtl of or_gate is
begin
   c_o <= a_i or b_i;
end architecture rtl;
