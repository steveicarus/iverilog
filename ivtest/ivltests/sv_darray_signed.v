module main;

  bit pass;
  bit signed [7:0] s8[];
  bit [7:0] u8[];
  string res, fmt;

  initial begin
    pass = 1'b1;
    s8 = new[2];
    u8 = new[2];
    s8[0] = -1;
    u8[0] = -1;

    fmt = "%0d";
    $display(fmt, s8[0]);
    $sformat(res, fmt, s8[0]);
    if (res != "-1") begin
      $display("Failed: expected '-1', got '%s'", res);
      pass = 1'b0;
    end

    $display(fmt, u8[0]);
    $swrite(res, u8[0]);
    if (res != "255") begin
      $display("Failed: expected '255', got '%s'", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule // main
