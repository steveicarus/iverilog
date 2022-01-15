module top;
  localparam enum_start = 2'd1;

  // This is an expression so the value just has to fit.
  enum logic [3:0] { first  = enum_start + 2'd0,
                     second = enum_start + 2'd1} my_enum;

  initial $display("PASSED");

endmodule
