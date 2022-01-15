library ieee;
use ieee.std_logic_1164.all;

entity nand104 is
  port (
    a_i : in std_logic_vector (103 downto 0);
    b_i : in std_logic_vector (103 downto 0);
    c_o : out std_logic_vector (103 downto 0)
  );
end entity nand104;

architecture rtl of nand104 is
begin
  c_o <= a_i nand b_i;
end architecture rtl;
