
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.work14_pkg.all;

entity work14_comp is

  generic (
    max_out_val : natural := 3;
    sample_parm : string := "test");

  port (
    clk_i : in  std_logic;
    val   : out std_logic_vector(f_log2_size(max_out_val)-1 downto 0));

end work14_comp;

architecture rtl of work14_comp is

begin  -- rtl

  foo : process(clk_i)
    begin
      if rising_edge(clk_i) then
        val <= std_logic_vector(to_unsigned(max_out_val, val'length));
      end if;
    end process;

end rtl;
