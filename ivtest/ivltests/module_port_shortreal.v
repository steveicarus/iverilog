// Check that shortreal module ports are supported

module M (
  input shortreal in,
  output shortreal out
);

  assign out = in * 10.1;

endmodule

module test;

  shortreal r;

  M m (
    .in (1.23),
    .out (r)
  );

  initial begin
    #1
    if (r == 12.423) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
