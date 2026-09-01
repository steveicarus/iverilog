module vec4_stack_reuse;
  integer failed;
  integer idx;
  reg [12:0] value13;
  reg [12:0] other13;
  reg [12:0] saved13;
  reg [64:0] value65;
  reg [64:0] other65;
  reg [64:0] saved65;
  reg [256:0] value257;
  reg [256:0] other257;
  reg [256:0] mask257;
  reg [256:0] saved257;
  reg [256:0] disabled_value;

  function [256:0] mix257;
    input [256:0] left;
    input [256:0] right;
    input [256:0] mask;
    begin
      mix257 = (left ^ right) ^ mask;
    end
  endfunction

  task check;
    input [256:0] actual;
    input [256:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED vec4 stack reuse test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  // Disabling this task while it is waiting also discards the vector
  // arguments held by its caller.
  task hold_arguments;
    input [256:0] wide;
    input [64:0] mid;
    input [12:0] narrow;
    begin
      #100;
      disabled_value = wide ^ {{192{1'b0}}, mid}
                            ^ {{244{1'b0}}, narrow};
    end
  endtask

  initial begin
    failed = 0;
    value13 = 13'h123;
    other13 = 13'h456;
    value65 = {1'b1, 64'h0123456789abcdef};
    other65 = {1'b0, 64'hfedcba9876543210};
    value257 = {1'b1, {4{64'h0123456789abcdef}}};
    other257 = {1'b0, {4{64'hfedcba9876543210}}};
    mask257 = {257{1'b1}};

    // Repeatedly change stack depth and vector width. Comparisons and stores
    // discard stack values, while function/task calls keep several live.
    for (idx = 0; idx < 32; idx = idx + 1) begin
      check(mix257(value257, other257, mask257), {257{1'b0}},
            10*idx + 1);
      check(value13 ^ other13, 13'h575, 10*idx + 2);
      check(value257 ^ other257, {257{1'b1}}, 10*idx + 3);
      check(value65 ^ other65, {65{1'b1}}, 10*idx + 4);

      if ((value257 ^ other257) !== {257{1'b1}}) begin
        $display("FAILED vec4 stack reuse comparison %0d", idx);
        failed = failed + 1;
      end

      saved257 = value257;
      saved13 = value13;
      saved65 = value65;
      check(saved257, value257, 10*idx + 5);
      check(saved13, value13, 10*idx + 6);
      check(saved65, value65, 10*idx + 7);
    end

    disabled_value = {257{1'bx}};
    fork : cancellable
      begin
        hold_arguments(mix257(value257, other257, mask257),
                       value65 ^ other65, value13 ^ other13);
        failed = failed + 1;
      end
      begin
        #1;
        disable cancellable;
      end
    join

    check(disabled_value, {257{1'bx}}, 1000);
    check(mix257(value257, other257, mask257), {257{1'b0}}, 1001);
    check(value13 ^ other13, 13'h575, 1002);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
