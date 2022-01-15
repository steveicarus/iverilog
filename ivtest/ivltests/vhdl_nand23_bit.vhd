library ieee;
use ieee.numeric_bit.all;

entity nand23 is
  port (
    a_i : in bit_vector (22 downto 0);
    b_i : in bit_vector (22 downto 0);
    c_o : out bit_vector (22 downto 0)
  );
end entity nand23;

architecture rtl of nand23 is
begin
  c_o <= a_i nand b_i;
end architecture rtl;
