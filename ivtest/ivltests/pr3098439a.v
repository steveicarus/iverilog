// This file extends the original bug test case to explore all the
// forms of a signed left shift that are treated as special cases.
module test;
  reg pass;
  reg [8*40:1] str;
  integer s;

  initial begin
    pass = 1'b1;
    s = 1;

    $sformat(str, "%0d", ((0 << 1) + 1) * -1);
    if (str[8*2:1] !== "-1" || str[8*40:8*2+1] !== 0) begin
      $display("FAILED 1st test, expected \"-1\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", ((1 << 1) + 1) * -1);
    if (str[8*2:1] !== "-3" || str[8*40:8*2+1] !== 0) begin
      $display("FAILED 2nd test, expected \"-3\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", ((1 << s) + 1) * -1);
    if (str[8*2:1] !== "-3" || str[8*40:8*2+1] !== 0) begin
      $display("FAILED 3rd test, expected \"-3\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", ((s << 1) + 1) * -1);
    if (str[8*2:1] !== "-3" || str[8*40:8*2+1] !== 0) begin
      $display("FAILED 4th test, expected \"-3\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", ((s << 0) + 1) * -1);
    if (str[8*2:1] !== "-2" || str[8*40:8*2+1] !== 0) begin
      $display("FAILED 5th test, expected \"-2\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", ((s << 64) + 1'sd1) * -1'sd1);
    if (str[8*2:1] !== "-1" || str[8*40:8*2+1] !== 0) begin
      $display("FAILED 6th test, expected \"-1\", got %s", str);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
