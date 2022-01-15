library ieee;
use ieee.numeric_bit.all;

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

-- Declare and implement a 4-bit full-adder that uses the
-- 1-bit full-adder described above.
entity fa4 is
  port (va_i, vb_i: in bit_vector (3 downto 0);
        c_i: in bit;
        vs_o: out bit_vector (3 downto 0);
        c_o: out bit
        );
end entity fa4;

architecture fa4_rtl of fa4 is

  -- full 1-bit adder
  component fa1 is
    port (a_i, b_i, c_i: in bit;
          s_o, c_o: out bit);
  end component fa1;

  -- internal carry signals propagation
  signal c_int4, c_int3, c_int2, c_int1, c_int0: bit;

  begin

    -- carry in
    c_int0 <= c_i;

    -- slice 0
    s0: fa1 port map (c_i => c_int0,
                      a_i => va_i(0),
                      b_i => vb_i(0),
                      s_o => vs_o(0),
                      c_o => c_int1
                      );

    -- slice 1
    s1: fa1 port map (c_i => c_int1,
                      a_i => va_i(1),
                      b_i => vb_i(1),
                      s_o => vs_o(1),
                      c_o => c_int2
                      );

    -- slice 2
    s2: fa1 port map (c_i => c_int2,
                      a_i => va_i(2),
                      b_i => vb_i(2),
                      s_o => vs_o(2),
                      c_o => c_int3
                      );

    -- slice 3
    s3: fa1 port map (c_i => c_int3,
                      a_i => va_i(3),
                      b_i => vb_i(3),
                      s_o => vs_o(3),
                      c_o => c_int4
                      );

    -- carry out
    c_o <= c_int4;

end architecture fa4_rtl;
