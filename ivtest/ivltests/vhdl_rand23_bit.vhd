library ieee;
use ieee.numeric_bit.all;

entity rand23 is
  port (
    a_i : in bit_vector (0 to 22);
    b_i : in bit_vector (0 to 22);
    c_o : out bit_vector (0 to 22)
  );
end entity rand23;

architecture rtl of rand23 is
begin
  c_o <= a_i and b_i;
end architecture rtl;
