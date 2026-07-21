module M;

  reg r;
  reg rst_n;
  reg clk;
  reg sig_name_with_more_than_3_underscores_err;

  initial begin
    $lint_check("check_name");
  end

  dut u_dut (clk, rst_n);
  dut u0_dut (clk, rst_n);
  dut u1_dut (clk, rst_n);
  dut wrong_named_dut (clk, rst_n);

endmodule

module dut (input clk, rst_n);
endmodule 
