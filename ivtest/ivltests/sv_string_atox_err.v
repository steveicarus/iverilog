module top;
  integer res;
  string str;

  initial begin
    // Check atoi.
    str = "Not a value";
    res = str.atoi(1'b1);

    // Check atohex.
    str = "Not a value";
    res = str.atohex(1'b1);

    // Check atooct.
    str = "Not a value";
    res = str.atooct(1'b1);

    // Check atobin.
    str = "Not a value";
    res = str.atobin(1'b1);

    $display("FAILED");
  end
endmodule
