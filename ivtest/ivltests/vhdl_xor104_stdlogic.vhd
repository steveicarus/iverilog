library ieee;
use ieee.std_logic_1164.all;

entity xor104 is
  port (
    a_i : in std_logic_vector (103 downto 0);
    b_i : in std_logic_vector (103 downto 0);
    c_o : out std_logic_vector (103 downto 0)
  );
end entity xor104;

architecture rtl of xor104 is
begin
  c_o <= a_i xor b_i;
end architecture rtl;
