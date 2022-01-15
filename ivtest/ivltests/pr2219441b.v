module top;
  reg passed;
  reg [1:0] sel;
  reg [1:0] A;
  wire Z;

  parent parent(.sel(sel), .A(A), .Z(Z));

  initial begin
    // $monitor("1: %b, 0: %b", parent.child[1].Z, parent.child[0].Z);
    passed = 1'b1;
    sel = 2'b11;
    A = 2'b00;
    #1 if (Z !== 1'b0) begin
      $display("FAILED: selected both, expected 1'b0, got %b", Z);
      passed = 1'b0;
    end

    A = 2'b11;
    #1 if (Z !== 1'b1) begin
      $display("FAILED: selected both, expected 1'b1, got %b", Z);
      passed = 1'b0;
    end

    A = 2'b10;
    #1 if (Z !== 1'bx) begin
      $display("FAILED: selected both, expected 1'bx, got %b", Z);
      passed = 1'b0;
    end

    sel = 2'b00;
    #1 if (Z !== 1'bz) begin
      $display("FAILED: deselected, expected 1'bz, got %b", Z);
      passed = 1'b0;
    end

    sel = 2'b10;
    #1 if (Z !== 1'b1) begin
      $display("FAILED: selected (1), expected 1'b1, got %b", Z);
      passed = 1'b0;
    end

    sel = 2'b01;
    #1 if (Z !== 1'b0) begin
      $display("FAILED: selected (0), expected 1'b0, got %b", Z);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule

module parent(input[1:0] sel, input [1:0] A, inout Z);
  child child[1:0](.sel(sel), .A(A), .Z(Z));
endmodule

module child(input sel, input A, inout Z);
  assign Z = (sel) ? A : 1'bz;
endmodule
