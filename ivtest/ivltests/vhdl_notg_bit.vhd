library IEEE;
use IEEE.numeric_bit.all;


entity not_gate is
  port (
       a_i : in bit;    -- inputs
       c_o : out bit    -- output
       );
end entity not_gate;

architecture rtl of not_gate is
begin
   c_o <= not a_i;
end architecture rtl;
