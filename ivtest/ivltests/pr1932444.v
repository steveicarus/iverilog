module test;
  reg pass = 1'b1;

  reg array[1:0];
  reg [7:0] delay[1:0];
  integer i = 1, j = 0;

  initial begin
    delay[0] = 8'd4;
    delay[1] = 8'd6;
    array[j] <= #(delay[0]) 1'b0;
    array[i] <= #(delay[i]) 1'b1;
    #3;
    if (array[0] !== 1'bx) begin
      $display("FAILED: array[0] != 1'bx @ 3");
      pass = 1'b0;
    end

    #2;
    if (array[0] !== 1'b0) begin
      $display("FAILED: array[0] != 1'b0 @ 5");
      pass = 1'b0;
    end

    if (array[1] !== 1'bx) begin
      $display("FAILED: array[1] != 1'bx @ 5");
      pass = 1'b0;
    end

    #2;
    if (array[1] !== 1'b1) begin
      $display("FAILED: array[1] != 1'b1 @ 7");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");

  end
endmodule
