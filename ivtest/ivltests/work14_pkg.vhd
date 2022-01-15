
library ieee;
use ieee.std_logic_1164.all;

package work14_pkg is

  function f_log2_size (
    A : natural)
    return natural;
  component work14_comp
    generic (
      max_out_val : natural;
      sample_parm : string);
    port (
      clk_i : in  std_logic;
      val   : out std_logic_vector(f_log2_size(max_out_val)-1 downto 0));
  end component;

end work14_pkg;

package body work14_pkg is

  function f_log2_size (A : natural) return natural is
  begin
    for I in 1 to 64 loop               -- Works for up to 64 bits
      if (2**I >= A) then
        return(I);
      end if;
    end loop;
    return(63);
  end function f_log2_size;

end work14_pkg;
