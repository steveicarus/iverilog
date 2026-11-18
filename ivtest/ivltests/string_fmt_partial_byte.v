// Check that %s handles a value whose width is not a multiple of 8 bits.
// The partial top byte must be printed as its own character, and a 32761
// bit value must not write past the 4096 byte VPI result buffer.

module test;
  reg [32759:0] body;
  reg [32760:0] word [0:0];
  reg [8:0] narrow [0:0];
  reg [8*4096-1:0] text;
  reg [15:0] text2;
  reg failed;

  initial begin
    failed = 0;
    body = {4095{8'h41}};
    word[0] = {1'b1, body};
    narrow[0] = 9'h141;

    // Array words.
    $sformat(text, "%s", word[0]);
    if (text !== {8'h01, body}) begin
      $display("FAILED: wide array word with a partial top byte");
      failed = 1;
    end

    $sformat(text2, "%s", narrow[0]);
    if (text2 !== {8'h01, "A"}) begin
      $display("FAILED: got %h, expected 0141", text2);
      failed = 1;
    end

    // Expression evaluated on the thread stack.
    $sformat(text, "%s", {1'b1, body});
    if (text !== {8'h01, body}) begin
      $display("FAILED: expression with a partial top byte");
      failed = 1;
    end

    if (!failed) $display("PASSED");
  end
endmodule
