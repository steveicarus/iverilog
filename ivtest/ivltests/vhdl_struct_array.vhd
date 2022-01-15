library ieee;
use ieee.std_logic_1164.all;

entity foo_entity is
  port (
    i_low0: in std_logic_vector (3 downto 0);
    i_high0: in std_logic_vector (3 downto 0);
    i_low1: in std_logic_vector (3 downto 0);
    i_high1: in std_logic_vector (3 downto 0);
    o_low0: out std_logic_vector (3 downto 0);
    o_high0: out std_logic_vector (3 downto 0);
    o_low1: out std_logic_vector (3 downto 0);
    o_high1: out std_logic_vector (3 downto 0)
  );
end foo_entity;

architecture beh of foo_entity is

type word is record
  high: std_logic_vector (3 downto 0);
  low: std_logic_vector (3 downto 0);
end record;

type dword is array (1 downto 0) of word;

signal my_dword: dword;

begin
  -- inputs
  my_dword(0).low <= i_low0;
  my_dword(0).high <= i_high0;
  my_dword(1).low <= i_low1;
  my_dword(1).high <= i_high1;

  -- outputs
  o_low0 <= my_dword(0).low;
  o_high0 <= my_dword(0).high;
  o_low1 <= my_dword(1).low;
  o_high1 <= my_dword(1).high;

end beh;
