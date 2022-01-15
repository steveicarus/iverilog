library IEEE;
use IEEE.numeric_bit.all;


entity nand_gate is
  port (
       a_i : in bit;    -- inputs
       b_i : in bit;
       c_o : out bit    -- output
       );
end entity nand_gate;

architecture rtl of nand_gate is
begin
   c_o <= a_i nand b_i;
end architecture rtl;
