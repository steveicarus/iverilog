// Check that soft packed unions can not have default member values.

module test;

  typedef union soft {
    logic [31:0] w = 32'h1;
    logic [7:0] b;
  } data_t;

  data_t data;

endmodule
