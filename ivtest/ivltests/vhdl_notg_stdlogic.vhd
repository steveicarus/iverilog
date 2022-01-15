library IEEE;
use IEEE.std_logic_1164.all;


entity not_gate is
  port (
       a_i : in std_logic;    -- inputs
       c_o : out std_logic    -- output
       );
end entity not_gate;

architecture rtl of not_gate is
begin
   c_o <= not a_i;
end architecture rtl;
