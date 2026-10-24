// Check that forcing an unknown bit select from a signal leaves the net unchanged.

module test;

  wire [3:0] value = 4'ha;
  reg source;
  reg failed;

  initial begin
    failed = 1'b0;
    source = 1'b1;
    force value[1'bx] = source;
    force value[1'bz] = source;
    #1;
    if (value !== 4'ha) begin
      failed = 1'b1;
    end
    source = 1'b0;
    #1;
    if (value !== 4'ha) begin
      failed = 1'b1;
    end

    if (failed) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
