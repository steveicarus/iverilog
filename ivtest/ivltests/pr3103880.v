module test;
  reg pass;
  reg [8*40:1] str;
  reg [15:0] v;

  initial begin
    pass = 1'b1;
    v = 2;

    $sformat(str, "%0d", (v + 2 - 1) * 1);
    if (str[8*1:1] !== "3" || str[8*40:8*1+1] !== 0) begin
      $display("FAILED 1st test, expected \"3\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", 'd1 - 'd2 + v);
    if (str[8*1:1] !== "1" || str[8*40:8*1+1] !== 0) begin
      $display("FAILED 2nd test, expected \"1\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", v + (-1));
    if (str[8*1:1] !== "1" || str[8*40:8*1+1] !== 0) begin
      $display("FAILED 3rd test, expected \"1\", got %s", str);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
