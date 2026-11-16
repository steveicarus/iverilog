// Check compound assignments after an unrelated unknown array index.

module test;

  logic [7:0] value;
  logic words[0:1];
  integer index = 'bx;

  assign value[7:4] = 4'h3;

  initial begin
    value[3:0] = 1;
    words[index] = 1'b1;
    value[3:0] += 1;
    #1;
    if (value !== 8'h32) begin
      $display("FAILED: expected 32, got %h", value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
