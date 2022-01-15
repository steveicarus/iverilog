library IEEE;
use IEEE.std_logic_1164.all;


entity xor_gate is
  port (
       a_i : in std_logic;    -- inputs
       b_i : in std_logic;
       c_o : out std_logic    -- output
       );
end entity xor_gate;

architecture rtl of xor_gate is
begin
   c_o <= a_i xor b_i;
end architecture rtl;
