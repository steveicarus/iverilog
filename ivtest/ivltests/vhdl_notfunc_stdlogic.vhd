library ieee;
use ieee.std_logic_1164.all;

entity not_func is
  port (
    a_i : in  std_logic;
    c_o : out std_logic
  );
end not_func;

architecture rtl of not_func is

  function invert (
    i : std_logic
  ) return std_logic is
  begin
    return not i;
  end function invert;

begin
  c_o <= invert(a_i);
end architecture rtl;
