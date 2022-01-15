module top;
  reg pass = 1'b1;
  reg [7:0] result;

  initial begin
    result = {{0{1'b1}}, 2'b11};
    if (result != 8'h03) begin
      $display("FAILED: replication inside of concatenation");
      pass = 1'b0;
    end

    result = {2{{0{1'b1}}, 2'b11}};
    if (result != 8'h0f) begin
      $display("FAILED: replication inside of replication");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
