module top;
  reg pass;

  reg  [8:0] a;
  wire [7:0] res_a;

  reg [6:0] b;
  wire [7:0] res_b;

  reg signed [6:0] c;
  wire [7:0] res_c;

  assign res_a = Copy(a);
  assign res_b = Copy(b);
  assign res_c = Copy(c);

  initial begin
    pass = 1'b1;
    a = 9'h101;
    b = -7'd1;
    c = -7'd1;
    #1;

    if (res_a !== 8'h01) begin
      $display("Failed to crop a vector, got %b.", res_a);
      pass = 1'b0;
    end

    if (res_b !== 8'h7f) begin
      $display("Failed to zero extend an unsigned vector, got %b.", res_b);
      pass = 1'b0;
    end

    if (res_c !== 8'hff) begin
      $display("Failed to sign extend a signed vector, got %b.", res_c);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

  function [7:0] Copy;
    input [7:0] Value;
    begin
      Copy = Value;
    end
  endfunction
endmodule
