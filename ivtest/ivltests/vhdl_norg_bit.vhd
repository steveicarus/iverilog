library IEEE;
use IEEE.numeric_bit.all;


entity nor_gate is
  port (
       a_i : in bit;    -- inputs
       b_i : in bit;
       c_o : out bit    -- output
       );
end entity nor_gate;

architecture rtl of nor_gate is
begin
   c_o <= a_i nor b_i;
end architecture rtl;
