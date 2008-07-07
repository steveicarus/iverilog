--
-- Support routines for Icarus Verilog VHDL output
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package Verilog_Support is

  -- This routine implements $finish by terminating the simulation
  -- It is implemented via the VHPI interface
  procedure finish;
  attribute foreign of finish : procedure is "VHPIDIRECT finish";

  -- Routines to implement Verilog reduction operators
  function Reduce_OR(X : unsigned) return std_logic;
  
  
end Verilog_Support;

package body Verilog_Support is

  -- This is a dummy body to provide a default implementation
  -- if VHPI is not supported
  procedure finish is
  begin
    assert false severity failure;
  end finish;

  function Reduce_OR(X : unsigned) return std_logic is
  begin
    for I in X'range loop
      if X(I) = '1' then
        return '1';
      end if;
    end loop;
    return '0';
  end function;  
  
end Verilog_Support;
