library ieee;
use ieee.numeric_bit.all;

entity udiv23 is
  port (
    a_i : in unsigned (22 downto 0);
    b_i : in unsigned (22 downto 0);
    c_o : out unsigned (22 downto 0)
  );
end entity udiv23;

architecture rtl of udiv23 is
begin
  c_o <= a_i / b_i;
end architecture rtl;
