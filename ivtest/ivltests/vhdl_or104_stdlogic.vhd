library ieee;
use ieee.std_logic_1164.all;

entity or104 is
  port (
    a_i : in std_logic_vector (103 downto 0);
    b_i : in std_logic_vector (103 downto 0);
    c_o : out std_logic_vector (103 downto 0)
  );
end entity or104;

architecture rtl of or104 is
begin
  c_o <= a_i or b_i;
end architecture rtl;
