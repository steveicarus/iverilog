// Check that literal arguments to system tasks may be wider than the
// fixed size buffer the vvp code generator used to format them in.
module top;
  reg passed;

  reg [4089:0]  v4090;
  reg [4095:0]  v4096;
  reg [4096:0]  v4097;
  reg [8191:0]  v8192;
  reg [19999:0] v20000;

  reg [8*20000:1] lit_str, sig_str;

  initial begin
    passed = 1'b1;

    v4090 = 4090'hx5a;
    $sformat(lit_str, "%b", 4090'hx5a);
    $sformat(sig_str, "%b", v4090);
    if (lit_str !== sig_str) begin
      $display("FAILED: 4090 bit literal formatted incorrectly");
      passed = 1'b0;
    end

    v4096 = 4096'hz3c;
    $sformat(lit_str, "%b", 4096'hz3c);
    $sformat(sig_str, "%b", v4096);
    if (lit_str !== sig_str) begin
      $display("FAILED: 4096 bit literal formatted incorrectly");
      passed = 1'b0;
    end

    v4097 = 4097'h1_dead_beef_cafe_f00d;
    $sformat(lit_str, "%b", 4097'h1_dead_beef_cafe_f00d);
    $sformat(sig_str, "%b", v4097);
    if (lit_str !== sig_str) begin
      $display("FAILED: 4097 bit literal formatted incorrectly");
      passed = 1'b0;
    end

    v8192 = 8192'h8000_0000_dead_beef;
    $sformat(lit_str, "%h", 8192'h8000_0000_dead_beef);
    $sformat(sig_str, "%h", v8192);
    if (lit_str !== sig_str) begin
      $display("FAILED: 8192 bit literal formatted incorrectly");
      passed = 1'b0;
    end

    v20000 = 20000'hx0123_4567_89ab_cdef;
    $sformat(lit_str, "%b", 20000'hx0123_4567_89ab_cdef);
    $sformat(sig_str, "%b", v20000);
    if (lit_str !== sig_str) begin
      $display("FAILED: 20000 bit literal formatted incorrectly");
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
