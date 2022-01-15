library ieee;
use ieee.std_logic_1164.all;

entity foo_entity is

  port(
    data_i  : in  std_logic_vector(1 downto 0);
    data_o, data_o2, data_o3  : out std_logic_vector(3 downto 0)
  );

end foo_entity;

architecture behaviour of foo_entity is

begin

  data_o <= "0001" when ( data_i="00" ) else
            "0010" when ( data_i="01" ) else
            "0100" when ( data_i="10" ) else
            "1000";

  -- test cases without the final 'else' statement
  data_o2 <= "0101" when ( data_i="01" );

  data_o3 <= "1100" when ( data_i="10" ) else
             "0011" when ( data_i="01" );

end behaviour;
