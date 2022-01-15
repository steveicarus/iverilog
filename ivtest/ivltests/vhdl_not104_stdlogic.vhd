library ieee;
use ieee.std_logic_1164.all;

entity not104 is
  port (
    a_i : in std_logic_vector (103 downto 0);
    c_o : out std_logic_vector (103 downto 0)
  );
end entity not104;

architecture rtl of not104 is
begin
  c_o <= not a_i;
end architecture rtl;
