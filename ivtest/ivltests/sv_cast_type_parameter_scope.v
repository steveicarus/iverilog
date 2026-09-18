// Check that a cast target type is resolved separately for each instance.

module M #(
  parameter type T = logic [3:0],
  parameter integer EXPECTED_WIDTH = 4
) (
  output reg failed
);

  initial begin
    failed = 1'b0;

    if ($bits(T'(0)) !== EXPECTED_WIDTH) begin
      $display("FAILED(%0d). Expected %0d, got %0d", `__LINE__,
               EXPECTED_WIDTH, $bits(T'(0)));
      failed = 1'b1;
    end
  end

endmodule

module test;

  wire failed4;
  wire failed8;

  M #(.T(logic [3:0]), .EXPECTED_WIDTH(4)) i4(.failed(failed4));
  M #(.T(logic [7:0]), .EXPECTED_WIDTH(8)) i8(.failed(failed8));

  initial begin
    #1;

    if (!failed4 && !failed8) begin
      $display("PASSED");
    end
  end

endmodule
