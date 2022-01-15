library IEEE;
use IEEE.std_logic_1164.all;


entity xnor_gate is
  port (
       a_i : in std_logic;    -- inputs
       b_i : in std_logic;
       c_o : out std_logic    -- output
       );
end entity xnor_gate;

architecture rtl of xnor_gate is
begin
   c_o <= a_i xnor b_i;
end architecture rtl;
