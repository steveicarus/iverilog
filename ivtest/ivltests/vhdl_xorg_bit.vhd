library IEEE;
use IEEE.numeric_bit.all;


entity xor_gate is
  port (
       a_i : in bit;    -- inputs
       b_i : in bit;
       c_o : out bit    -- output
       );
end entity xor_gate;

architecture rtl of xor_gate is
begin
   c_o <= a_i xor b_i;
end architecture rtl;
