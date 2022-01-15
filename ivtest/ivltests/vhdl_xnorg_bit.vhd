library IEEE;
use IEEE.numeric_bit.all;


entity xnor_gate is
  port (
       a_i : in bit;    -- inputs
       b_i : in bit;
       c_o : out bit    -- output
       );
end entity xnor_gate;

architecture rtl of xnor_gate is
begin
   c_o <= a_i xnor b_i;
end architecture rtl;
