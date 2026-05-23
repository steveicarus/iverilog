// Check that drive strength can be specified between the net type and the data
// type in a net declaration and that vector gate arrays resolve strengths
// correctly.

module test;

  reg [7:0] pullval;
  wire (weak0, weak1) [7:0] value = pullval;

  reg [7:0] en0;
  reg [7:0] en1;
  reg failed = 1'b0;

  `define check(expr, val) \
    if ((expr) !== (val)) begin \
      $display("FAILED(%0d): `%s`, expected %0h, got %0h", `__LINE__, \
               `"expr`", (val), (expr)); \
      failed = 1'b1; \
    end

  buf (highz0, strong1) drive0 [7:0] (value, en0);
  not (strong0, highz1) drive1 [7:0] (value, en1);

  initial begin
    en0 = 8'h00;
    en1 = 8'h00;

    pullval = 8'hff;
    #1 `check(value, 8'hff)

    pullval = 8'h00;
    #1 `check(value, 8'h00)

    en0 = 8'haa;
    pullval = 8'hff;
    #1 `check(value, 8'hff)

    pullval = 8'h00;
    #1 `check(value, 8'haa)

    en0 = 8'h00;
    en1 = 8'hff;
    pullval = 8'hff;
    #1 `check(value, 8'h00)

    pullval = 8'h00;
    #1 `check(value, 8'h00)

    en0 = 8'hff;
    en1 = 8'hff;
    pullval = 8'hff;
    #1 `check(value, 8'hxx)

    pullval = 8'h00;
    #1 `check(value, 8'hxx)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
