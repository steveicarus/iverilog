// Regression test for bug reported by Orson on 24-Apr-15 via iverilog_devel.

module test();

localparam value = $ivlh_to_unsigned((1000 / ($signed(13'd50))), 12);

initial begin
  $display("%d", value);
  if (value === 20)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

