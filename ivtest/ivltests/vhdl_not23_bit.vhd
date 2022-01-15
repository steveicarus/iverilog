library ieee;
use ieee.numeric_bit.all;

entity not23 is
  port (
    a_i : in bit_vector (22 downto 0);
    c_o : out bit_vector (22 downto 0)
  );
end entity not23;

architecture rtl of not23 is
begin
  c_o <= not a_i;
end architecture rtl;
