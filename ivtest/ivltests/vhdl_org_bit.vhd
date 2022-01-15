library IEEE;
use IEEE.numeric_bit.all;


entity or_gate is
  port (
       a_i : in bit;    -- inputs
       b_i : in bit;
       c_o : out bit    -- output
       );
end entity or_gate;

architecture rtl of or_gate is
begin
   c_o <= a_i or b_i;
end architecture rtl;
