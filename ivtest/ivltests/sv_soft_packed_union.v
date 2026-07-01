// Check that soft packed unions can have members with different widths.

module test;

  bit failed = 1'b0;

  typedef union soft packed {
    logic [31:0] w;
    logic [15:0] h;
    logic [7:0] b;
  } data_t;

  typedef union soft {
    logic [31:0] w;
    logic [7:0] b;
  } implicit_packed_t;

  typedef union soft packed {
    logic [15:0] w;
    union soft {
      logic [7:0] b;
      logic [3:0] n;
    } part;
  } nested_t;

  data_t data;
  implicit_packed_t implicit_packed;
  nested_t nested;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  initial begin
    data.w = 32'h12345678;
    `check($bits(data), 32);
    `check($bits(data_t), 32);
    `check(data.w, 32'h12345678);
    `check(data.h, 16'h5678);
    `check(data.b, 8'h78);

    data.h = 16'habcd;
    `check(data.w, 32'h1234abcd);
    `check(data.h, 16'habcd);
    `check(data.b, 8'hcd);

    data.b = 8'hef;
    `check(data.w, 32'h1234abef);
    `check(data.h, 16'habef);
    `check(data.b, 8'hef);

    implicit_packed.w = 32'hf0e1d2c3;
    `check($bits(implicit_packed_t), 32);
    `check(implicit_packed.b, 8'hc3);

    implicit_packed.b = 8'h34;
    `check(implicit_packed.w, 32'hf0e1d234);

    nested.w = 16'hbeef;
    `check($bits(nested_t), 16);
    `check($bits(nested.part), 8);
    `check(nested.part.b, 8'hef);
    `check(nested.part.n, 4'hf);

    nested.part.n = 4'ha;
    `check(nested.w, 16'hbeea);
    `check(nested.part.b, 8'hea);

    nested.part.b = 8'h55;
    `check(nested.w, 16'hbe55);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
