-- In this test, we declare a component in the "mypackage" package
-- and show that it can be referenced within the package namespace.
-- it also shows the usage of subtypes, constants and signals
-- expressed in terms of defined subtypes

library ieee;
use ieee.numeric_bit.all;

package mypackage is

  -- trivial sub type
  subtype Myrange_t is integer range 0 to 4;

  -- some constants
  constant ZERO: Myrange_t := 0;
  constant ONE: Myrange_t := 1;
  constant TWO: Myrange_t := 2;
  constant THREE: Myrange_t := 3;
  constant FOUR: Myrange_t := 4;

  -- another subtype
  subtype AdderWidth_t is bit_vector (THREE downto ZERO);
  subtype CarryWidth_t is bit_vector (THREE+1 downto ZERO);

  -- full 1-bit adder
  component fa1 is
    port (a_i, b_i, c_i: in bit;
          s_o, c_o: out bit);
  end component fa1;
end package mypackage;

-- Declare and implement a 4-bit full-adder that uses the
-- 1-bit full-adder described above.

use work.mypackage.all;
entity fa4 is
  port (va_i, vb_i: in AdderWidth_t;
        c_i: in bit;
        vs_o: out AdderWidth_t;
        c_o: out bit
        );
end entity fa4;

architecture fa4_rtl of fa4 is

-- auxiliary signal for carry
signal c_int: CarryWidth_t;

begin

    -- carry in
    c_int(ZERO) <= c_i;

    -- slice 0
    s0: fa1 port map (c_i => c_int(ZERO),
                      a_i => va_i(ZERO),
                      b_i => vb_i(ZERO),
                      s_o => vs_o(ZERO),
                      c_o => c_int(ONE)
                      );

    -- slice 1
    s1: fa1 port map (c_i => c_int(ONE),
                      a_i => va_i(ONE),
                      b_i => vb_i(ONE),
                      s_o => vs_o(ONE),
                      c_o => c_int(TWO)
                      );

    -- slice 2
    s2: fa1 port map (c_i => c_int(TWO),
                      a_i => va_i(TWO),
                      b_i => vb_i(TWO),
                      s_o => vs_o(TWO),
                      c_o => c_int(THREE)
                      );

    -- slice 3
    s3: fa1 port map (c_i => c_int(THREE),
                      a_i => va_i(THREE),
                      b_i => vb_i(THREE),
                      s_o => vs_o(THREE),
                      c_o => c_int(FOUR)
                      );

    -- carry out
    c_o <= c_int(FOUR);

end architecture fa4_rtl;

-- Declare a 1-bit full-adder.
entity fa1 is
  port (a_i, b_i, c_i: in bit;
        s_o, c_o: out bit
        );
end entity fa1;

architecture fa1_rtl of fa1 is
begin
  s_o <= a_i xor b_i xor c_i;
  c_o <= (a_i and b_i) or (c_i and (a_i xor b_i));
end architecture fa1_rtl;
