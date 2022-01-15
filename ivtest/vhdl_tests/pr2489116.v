module top();
  tri tdo, tdo_out;
  reg tdo_oe;

  assign tdo = tdo_oe ? tdo_out : 1'bz;

  assign tdo_out = 1'b1;

  // Check undriven value is Z
  initial begin
    tdo_oe = 0;
    #1;
    if (tdo !== 1'bz) begin
      $display("FAILED -- tdo not z when tdo_oe = 0");
      $finish;
    end

    tdo_oe = 1;
    #1;
    #1;
    if (tdo !== 1'b1) begin
      $display("FAILED -- tdo not tdo_out when tdo_oe = 0");
      $finish;
    end

    $display("PASSED");
  end

endmodule // top
