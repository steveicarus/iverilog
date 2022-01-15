library ieee;
use ieee.std_logic_1164.all;

package diq_pkg is


component Add_Synth
generic (n: integer);
port (a,b: in std_logic_vector (n-1 downto 0);
      cin: in std_logic;
      comp : out std_logic;
      sum : out std_logic_vector (n-1 downto 0) );
end component;

component Inc_Synth
generic (n: integer);
port (a: in std_logic_vector (n-1 downto 0);
      sum : out std_logic_vector (n-1 downto 0) );
end component;

end package;

library ieee;
use ieee.std_logic_1164.all;
use work.diq_pkg.all;

entity diq_array is
generic (width: integer := 8; size: integer := 7);
port (clk,reset: in std_logic;
	  din,bin,xin: in std_logic_vector (width-1 downto 0);
	  lin: in std_logic_vector (2 downto 0);
	  lout: out std_logic_vector (2 downto 0);
	  dout,bout,xout: out std_logic_vector (width-1 downto 0) );
end diq_array;

architecture systolic of diq_array is

component diq
generic (n: integer );
port (clk,reset: in std_logic;
     lin: in std_logic_vector (2 downto 0);
	  din,bin,xin: in std_logic_vector (n-1 downto 0);
	  lout: out std_logic_vector (2 downto 0);
	  dout,bout,xout: out std_logic_vector (n-1 downto 0) );
end component;

type path is array (0 to size) of std_logic_vector (width-1 downto 0);
type l_path is array (0 to size) of std_logic_vector (2 downto 0);
signal x_path, d_path, b_path: path;
signal l_int: l_path;
begin
gen_arrays: for i in 0 to size-1 generate
each_array: diq generic map (n => width)
                port map (clk => clk, din => d_path(i), bin => b_path(i), reset => reset,
                          xin => x_path(i), lin => l_int(i),
                          dout => d_path(i+1), bout => b_path(i+1),
                          xout => x_path(i+1), lout => l_int(i+1) );
end generate;
d_path(0) <= din;
b_path(0) <= bin;
x_path(0) <= xin;
l_int(0) <= lin;
dout <= d_path(size);
bout <= b_path(size);
xout <= x_path(size);
lout <= l_int(size);
end systolic;

library ieee;
use ieee.std_logic_1164.all;
use work.diq_pkg.all;

entity diq is
generic (n: integer := 8);
port (clk, reset: in std_logic;
	  din,bin,xin: in std_logic_vector (n-1 downto 0);
	  lin: in std_logic_vector (2 downto 0);
	  dout,bout,xout: out std_logic_vector (n-1 downto 0);
	  lout: out std_logic_vector (2 downto 0) );
end diq;

architecture diq_wordlevel of diq is


signal b_int, d_int, x_int, x_inv: std_logic_vector (n-1 downto 0);
signal l_int, l_inc: std_logic_vector (2 downto 0);
signal sel: std_logic;
signal zero,uno: std_logic;
begin
d_reg: process(clk,reset)
begin
if reset = '1' then
	d_int <= (others => '0');
elsif (clk'event and clk = '1') then
	d_int <= din;
end if;
end process;

l_reg: process(clk,reset)
begin
if reset = '1' then
	l_int <= (others => '0');
elsif (clk'event and clk = '1') then
	l_int <= lin;
end if;
end process;


b_reg: process(clk,reset)
begin
if reset = '1' then
	b_int <= (others => '0');
elsif (clk'event and clk = '1') then
	b_int <= bin;
end if;
end process;

x_reg: process(clk,reset)
begin
if reset = '1' then
	x_int <= (others => '0');
elsif (clk'event and clk = '1') then
	x_int <= xin;
end if;
end process;


zero <= '0';
uno <= '1';
addition: Add_Synth generic map (n => n)
                  port map (a => b_int, b => d_int, cin => zero, comp => open, sum => bout);
x_inv <= not x_int;
comparison: Add_Synth generic map (n => n)
                  port map (a => b_int, b => x_inv, cin => uno, comp => sel, sum => open);
incrementer: Inc_Synth generic map (n => 3)
                  port map (a => l_int, sum => l_inc);
-- outputs
lout <= l_inc when (sel = '1') else l_int;
dout <= d_int;
xout <= x_int;
end diq_wordlevel;



library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity Inc_Synth is
	generic (n: integer := 8);
	port (a: in std_logic_vector (n-1 downto 0);
		  sum: out std_logic_vector (n-1 downto 0)
		  );
end Inc_Synth;

architecture compact_inc of Inc_Synth is
signal cx: std_logic_vector (n downto 0);
begin
cx <= ('0' & a) +  '1';
sum <= cx (n-1 downto 0);
end compact_inc;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity Add_Synth is
	generic (n: integer := 8);
	port (a, b: in std_logic_vector (n-1 downto 0);
		  sum: out std_logic_vector (n-1 downto 0);
		  cin: in std_logic;
		  comp: out std_logic );
end Add_Synth;

architecture compact of Add_Synth is
signal cx: std_logic_vector (n downto 0);
begin
cx <= ('0' & a) + ('0' & b) + cin;
sum <= cx (n-1 downto 0);
comp <= cx(n-1);
end compact;
