module test;
  reg pass;
  reg [8*40:1] str;
  integer s;
  reg [31:0] su;
  integer res;

  initial begin
    pass = 1'b1;
    s = 2000;
    su = 2000;

    res = s + (1 << 3) - 1;
    if (res !== 2007) begin
      $display("FAILED first term << (s), expected 2007, got %d", res);
      pass = 1'b0;
    end
    res = su + (1 << 3) - 1;
    if (res !== 2007) begin
      $display("FAILED first term << (su), expected 2007, got %d", res);
      pass = 1'b0;
    end

    res = s + (16 >> 1) - 1;
    if (res !== 2007) begin
      $display("FAILED first term >> (s), expected 2007, got %d", res);
      pass = 1'b0;
    end
    res = su + (16 >> 1) - 1;
    if (res !== 2007) begin
      $display("FAILED first term >> (su), expected 2007, got %d", res);
      pass = 1'b0;
    end

    res = (s + (1 << 3) - 1) * 16000;
    if (res !== 32112000) begin
      $display("FAILED second term << (s), expected 32112000, got %d", res);
      pass = 1'b0;
    end
    res = (su + (1 << 3) - 1) * 16000;
    if (res !== 32112000) begin
      $display("FAILED second term << (su), expected 32112000, got %d", res);
      pass = 1'b0;
    end

    res = (s + (16 >> 1) - 1) * 16000;
    if (res !== 32112000) begin
      $display("FAILED second term >> (s), expected 32112000, got %d", res);
      pass = 1'b0;
    end
    res = (su + (16 >> 1) - 1) * 16000;
    if (res !== 32112000) begin
      $display("FAILED second term >> (su), expected 32112000, got %d", res);
      pass = 1'b0;
    end

    $sformat(str, "%0d", s + (1 << 3) - 1);
    if (str[8*4:1] !== "2007" || str[8*40:8*4+1] !== 0) begin
      $display("FAILED first string << (s), expected \"2007\", got %s", str);
      pass = 1'b0;
    end
    $sformat(str, "%0d", su + (1 << 3) - 1);
    if (str[8*4:1] !== "2007" || str[8*40:8*4+1] !== 0) begin
      $display("FAILED first string << (su), expected \"2007\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", s + (16 >> 1) - 1);
    if (str[8*4:1] !== "2007" || str[8*40:8*4+1] !== 0) begin
      $display("FAILED first string >> (s), expected \"2007\", got %s", str);
      pass = 1'b0;
    end
    $sformat(str, "%0d", su + (16 >> 1) - 1);
    if (str[8*4:1] !== "2007" || str[8*40:8*4+1] !== 0) begin
      $display("FAILED first string >> (su), expected \"2007\", got %s", str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", (s + (1 << 3) - 1) * 16000);
    if (str[8*8:1] !== "32112000" || str[8*40:8*8+1] !== 0) begin
      $display("FAILED second string << (s), expected \"32112000\", got %s",
               str);
      pass = 1'b0;
    end
    $sformat(str, "%0d", (su + (1 << 3) - 1) * 16000);
    if (str[8*8:1] !== "32112000" || str[8*40:8*8+1] !== 0) begin
      $display("FAILED second string << (su), expected \"32112000\", got %s",
               str);
      pass = 1'b0;
    end

    $sformat(str, "%0d", (s + (16 >> 1) - 1) * 16000);
    if (str[8*8:1] !== "32112000" || str[8*40:8*8+1] !== 0) begin
      $display("FAILED second string >> (s), expected \"32112000\", got %s",
               str);
      pass = 1'b0;
    end
    $sformat(str, "%0d", (su + (16 >> 1) -1) * 16000);
    if (str[8*8:1] !== "32112000" || str[8*40:8*8+1] !== 0) begin
      $display("FAILED second string >> (su), expected \"32112000\", got %s",
               str);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
