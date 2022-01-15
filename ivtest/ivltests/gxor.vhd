library ieee;
use ieee.std_logic_1164.all;

entity gxor is
port (a, b: in std_logic;
             z   : out std_logic);
end gxor;

architecture gxor_rtl of gxor is
begin
   z <= a xor b;

end architecture gxor_rtl;

library ieee;
use ieee.std_logic_1164.all;

entity gxor_reduce is
  generic (half_width: integer := 4);
  port (a: in std_logic_vector (2*half_width-1 downto 0);
        ar: out std_logic);

end gxor_reduce;

architecture gxor_reduce_rtl of gxor_reduce is

component gxor is
  port (a, b: in std_logic;
             z   : out std_logic);
end component;

--type path is array (0 to size/2) of std_logic;
signal x_int: std_logic_vector (2*half_width downto 0);

begin
  x_int(2*half_width) <= '0'; -- MSB
gen_xor: for i in 2*half_width downto 1 generate
  each_gate: gxor port map (a => x_int(i), b => a(i-1), z => x_int(i-1) );
end generate;

ar <= x_int(0);

end architecture gxor_reduce_rtl;
