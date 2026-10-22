// Check strict continuous assignment evaluation of a real function.

module test;

  reg [7:0] value, scale, offset;
  reg [7:0] first_result;
  wire [7:0] result = scaled(value) + offset;

  function real scaled;
    input [7:0] arg;
    scaled = arg * scale + 0.5;
  endfunction

  initial begin
    value = 1;
    scale = 2;
    offset = 0;
    #1 first_result = result;

    scale = 4;
    offset = 1;
    #1;
    if (first_result !== 3 || result !== 6) begin
      $display("FAILED: got %0d, %0d", first_result, result);
    end else begin
      $display("PASSED");
    end
  end

endmodule
