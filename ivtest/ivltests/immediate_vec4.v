module immediate_vec4;
  integer failed;
  reg [0:0] u1;
  reg [30:0] u31;
  reg [31:0] u32;
  reg [32:0] u33;
  reg [63:0] u64;
  reg [64:0] u65;
  reg [126:0] u127;
  reg signed [64:0] s65;
  reg [64:0] nb65;
  reg [64:0] result65;
  reg cmp;

  task check;
    input [126:0] actual;
    input [126:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED immediate vec4 test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;

    // Exercise immediate construction on both sides of the native-word
    // boundary. The upper bits of an immediate wider than 32 bits must be 0.
    u1 = 1'd1;
    u31 = 31'h5a5a5a5;
    u32 = 32'h89abcdef;
    u33 = 33'h089abcdef;
    u64 = 64'h0000000089abcdef;
    u65 = 65'h0_00000000_89abcdef;
    u127 = 127'd17;
    check(u1, 127'd1, 1);
    check(u31, 127'h5a5a5a5, 2);
    check(u32, 127'h89abcdef, 3);
    check(u33, 127'h089abcdef, 4);
    check(u64, 127'h89abcdef, 5);
    check(u65, 127'h89abcdef, 6);
    check(u127, 127'd17, 7);

    // X and Z are encoded in a separate immediate bit plane.
    u127 = 127'h0xz1;
    check(u127[126:12], 127'd0, 8);
    check(u127[11:8], 4'hx, 9);
    check(u127[7:4], 4'hz, 10);
    check(u127[3:0], 4'h1, 11);

    // Exercise each arithmetic and comparison opcode that consumes an
    // immediate vector directly, including X/Z inputs and wide zero fill.
    u65 = 65'd17;
    result65 = u65 + 65'd5;
    check(result65, 65'd22, 12);
    result65 = u65 - 65'd5;
    check(result65, 65'd12, 13);
    result65 = u65 * 65'd5;
    check(result65, 65'd85, 14);
    result65 = u65 + 65'h1x;
    check(result65, {65{1'bx}}, 15);
    result65 = u65 - 65'h1z;
    check(result65, {65{1'bx}}, 16);
    result65 = u65 * 65'h1x;
    check(result65, {65{1'bx}}, 17);
    if (u65 != 65'd17) begin
      $display("FAILED immediate vec4 test 18");
      failed = failed + 1;
    end
    if (u65 == 65'd18) begin
      $display("FAILED immediate vec4 test 19");
      failed = failed + 1;
    end
    cmp = u65 == 65'h1x;
    if (cmp !== 1'bx) begin
      $display("FAILED immediate vec4 test 20");
      failed = failed + 1;
    end
    cmp = u65 != 65'h1z;
    if (cmp !== 1'bx) begin
      $display("FAILED immediate vec4 test 21");
      failed = failed + 1;
    end
    if ((u65 < 65'd18) !== 1'b1) begin
      $display("FAILED immediate vec4 test 22");
      failed = failed + 1;
    end
    if ((u65 < 65'h1x) !== 1'bx) begin
      $display("FAILED immediate vec4 test 23");
      failed = failed + 1;
    end
    s65 = -65'sd2;
    if ((s65 < 65'sd1) !== 1'b1) begin
      $display("FAILED immediate vec4 test 24");
      failed = failed + 1;
    end

    // Keep the high operand dynamic so these lower constants use %concati.
    u1 = 1'b1;
    u65 = {u1, 64'd5};
    check(u65[64], 1'b1, 25);
    check(u65[63:0], 64'd5, 26);
    u127 = {u1, 126'd9};
    check(u127[126], 1'b1, 27);
    check(u127[125:0], 126'd9, 28);

    // This constant nonblocking assignment is fused into %assigni/vec4.
    nb65 = 65'd0;
    nb65 <= #1 65'd23;
    #2;
    check(nb65, 65'd23, 29);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
