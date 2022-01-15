library ieee;
use ieee.std_logic_1164.all;

package work6 is
  -- full 1-bit adder
  component fa1 is
    port (a_i, b_i, c_i: in std_logic;
          s_o, c_o: out std_logic);
  end component fa1;

  -- D-type flip flop
  component fdc is
    port (clk: in std_logic;
	      reset: in std_logic;
          d: in std_logic;
          q: out std_logic);
  end component;

  -- doing nothing at the moment
  constant N: integer range 0 to 16 := 4;

end package work6;

-- a 1-bit Moore-type adder to be used in
-- a serial adder FSM-driven architecture

--            ________        _____
--    a_i -->|       |------>|D   Q|---> s_o
--    b_i -->|  FA1  |       |     |
--           |       |---    |     |
--      ---> |_______|   |   |_____|
--rst __|_______________________|
--      |            |   |
--      |            |   |
--      |            |   |      ______
--      |            |    ---->|D   Q|---
--      |            |         |     |   |
--      |            |         |     |   |
--      |            |         |_____|   |
--      |            |____________|      |
--      |________________________________|
--

library ieee;
use ieee.std_logic_1164.all;
use work.work6.all;

entity sa1 is
  port (clk, reset: in std_logic;
        a_i, b_i: in std_logic;
        s_o: out std_logic
        );
end entity sa1;

architecture sa1_rtl of sa1 is
signal sum, carry, carry_reg: std_logic;
begin

 a1: fa1 port map (c_i => carry_reg,
                  a_i => a_i,
                  b_i => b_i,
                  s_o => sum,
                  c_o => carry
                  );

 f1: fdc port map (clk => clk, reset => reset, d => sum,   q => s_o);
 f2: fdc port map (clk => clk, reset => reset, d => carry, q => carry_reg);

end architecture sa1_rtl;
-- a one bit full adder written according to
-- textbook's boolean equations

library ieee;
use ieee.std_logic_1164.all;

entity fa1 is
  port (a_i, b_i, c_i: in std_logic;
        s_o, c_o: out std_logic
        );
end entity fa1;

architecture fa1_rtl of fa1 is
begin

  s_o <= a_i xor b_i xor c_i;
  c_o <= (a_i and b_i) or (c_i and (a_i xor b_i));

end architecture fa1_rtl;-- a D-type flip-flop with synchronous reset

library ieee;
use ieee.std_logic_1164.all;


entity fdc is
port (clk: in std_logic;
	  reset: in std_logic;
      d: in std_logic;
      q: out std_logic
);
end fdc;

architecture fdc_rtl of fdc is
begin

i_finish: process (clk)
begin
   if (clk'event and clk = '1') then
     if (reset = '1') then
	    q <= '0';
	  else
        q <= d;
	 end if;
   end if;
end process;

end fdc_rtl;
