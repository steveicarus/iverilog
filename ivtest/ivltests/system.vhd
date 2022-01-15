-- This system does nothing useful
-- It takes input X and this is registered internally
-- It computes x+1 and x+const independently
-- The output is computed as (x+const)-(x+1)=const-1
-- so the higher level modifies C and then C-1 is returned


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity Const_system is
generic (C: in integer := 500);
port (clk, reset: in std_logic;
	  x: in std_logic_vector (7 downto 0);
	  y: out std_logic_vector (10 downto 0) );
end Const_system;

library ieee;
use ieee.std_logic_1164.all;

entity Add is
	generic (n: integer := 8);
	port (a, b: in std_logic_vector (n-1 downto 0);
		  sum: out std_logic_vector (n-1 downto 0);
		  cin: in std_logic );
end Add;

library ieee;
use ieee.std_logic_1164.all;

entity Inc is
	generic (n: integer := 8);
	port (a: in std_logic_vector (n-1 downto 0);
		  sum: out std_logic_vector (n-1 downto 0)
		  );
end Inc;

library ieee;
use ieee.std_logic_1164.all;

entity Reg_N is
	generic (n: integer := 4);
	port (clk, reset: in std_logic;
	      a: in std_logic_vector (n-1 downto 0);
		    a_reg: out std_logic_vector (n-1 downto 0) );
end Reg_N;

architecture System_rtl of Const_system is

-- Register component
component Reg_N is
	generic (n: integer := 4);
	port (clk, reset: in std_logic;
	      a: in std_logic_vector (n-1 downto 0);
		    a_reg: out std_logic_vector (n-1 downto 0) );
end component;

-- incrementer component
component Inc is
	generic (n: integer := 8);
	port (a: in std_logic_vector (n-1 downto 0);
		  sum: out std_logic_vector (n-1 downto 0)
		  );
end component;

-- adder component
component Add is
	generic (n: integer := 8);
	port (a, b: in std_logic_vector (n-1 downto 0);
		  sum: out std_logic_vector (n-1 downto 0);
		  cin: in std_logic );
end component;


signal x_int: std_logic_vector (7 downto 0);
signal x_inc: std_logic_vector (7 downto 0);
signal x_sum: std_logic_vector (10 downto 0);
signal x_ext: std_logic_vector (10 downto 0);
signal x_inv: std_logic_vector (10 downto 0);
signal x_dif: std_logic_vector (10 downto 0);

signal zero, one: std_logic;

signal const: std_logic_vector (10 downto 0);
begin

const <= conv_std_logic_vector (C, 11);

-- connstant bit 0, 1
zero <= '0';
one  <= '1';

-- registering input X
RegX: Reg_N generic map (n => 8)
            port map ( clk => clk, reset => reset, a => x, a_reg => x_int);

-- Incrementing input x_int
incrementer: Inc generic map (n => 8)
                  port map (a => x_int, sum => x_inc);  -- x + 1

-- forming 1's complement of x+1
x_inv <= "111" & not x_inc;

x_ext <= "000" & x_int;
-- adding constant to x_int
addition: Add generic map (n => 11)
                port map (a => x_ext, b => const, cin => zero, sum => x_sum); -- x + 1000

-- this should get x+1000-(x+1) = 1000-1 = 999
subtraction: Add generic map (n => 11)
                  port map (a => x_sum, b => x_inv, cin => one, sum => x_dif);

-- registering output X
RegY: Reg_N generic map (n => 11)
            port map ( clk => clk, reset => reset, a => x_dif, a_reg => y);


end System_rtl;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

architecture Add_rtl of Add is
signal cx: std_logic_vector (n downto 0);
begin
cx <= ('0' & a) + ('0' & b) + cin;
sum <= cx (n-1 downto 0);
end Add_rtl;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

architecture Inc_rtl of Inc is
signal cx: std_logic_vector (n downto 0);
begin
cx <= ('0' & a) +  '1';
sum <= cx (n-1 downto 0);
end Inc_rtl;

library ieee;
use ieee.std_logic_1164.all;

architecture Reg_rtl of Reg_N is
begin

My_register: process (clk, reset)
begin
  if (reset = '1') then
    a_reg <= (others => '0');
  elsif (clk'event and clk = '1') then
	  a_reg <= a;
	end if;
end process;

end Reg_rtl;
