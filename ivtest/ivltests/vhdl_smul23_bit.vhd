library ieee;
use ieee.numeric_bit.all;

entity smul23 is
  port (
    a_i : in signed (22 downto 0);
    b_i : in signed (22 downto 0);
    c_o : out signed (45 downto 0)
  );
end entity smul23;

architecture rtl of smul23 is
begin
  c_o <= a_i * b_i;
end architecture rtl;
