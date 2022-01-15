library ieee;
use ieee.numeric_bit.all;

entity or23 is
  port (
    a_i : in bit_vector (22 downto 0);
    b_i : in bit_vector (22 downto 0);
    c_o : out bit_vector (22 downto 0)
  );
end entity or23;

architecture rtl of or23 is
begin
  c_o <= a_i or b_i;
end architecture rtl;
