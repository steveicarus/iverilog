library IEEE;
use IEEE.std_logic_1164.all;


entity nand_gate is
  port (
       a_i : in std_logic;    -- inputs
       b_i : in std_logic;
       c_o : out std_logic    -- output
       );
end entity nand_gate;

architecture rtl of nand_gate is
begin
   c_o <= a_i nand b_i;
end architecture rtl;
