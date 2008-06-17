--
-- VHPI support routines for VHDL output.
--

package Verilog_Support is
  procedure finish;
  attribute foreign of finish : procedure is "VHPIDIRECT finish";
end Verilog_Support;

package body Verilog_Support is
  procedure finish is
  begin
    assert false severity failure;
  end finish;
end Verilog_Support;
