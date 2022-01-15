module top;
  reg passed;
  reg [1:0] sel;
  reg [1:0] A;
  wire [1:0] Z;

  parent parent(.sel(sel), .A(A), .Z(Z));

  initial begin
    passed = 1'b1;
    sel = 2'b11;
    A = 2'b00;
    #1 if (Z !== 2'b00) begin
      $display("FAILED: selected, expected 2'b00, got %b", Z);
      passed = 1'b0;
    end

    A = 2'b10;
    #1 if (Z !== 2'b10) begin
      $display("FAILED: selected, expected 2'b10, got %b", Z);
      passed = 1'b0;
    end

    A = 2'b01;
    #1 if (Z !== 2'b01) begin
      $display("FAILED: selected, expected 2'b01, got %b", Z);
      passed = 1'b0;
    end

    sel = 2'b00;
    #1 if (Z !== 2'bzz) begin
      $display("FAILED: deselected, expected 2'bzz, got %b", Z);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule

module parent(input[1:0] sel, input [1:0] A, inout [1:0] Z);
  child child[1:0](.sel(sel), .A(A), .Z(Z));
endmodule

module child(input sel, input A, inout Z);
  assign Z = (sel) ? A : 1'bz;
endmodule
