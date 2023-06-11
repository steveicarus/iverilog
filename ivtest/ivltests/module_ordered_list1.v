// Check that it is possible to omit trailing module ports in a ordered list
// connection if the trailing port has a default value.

module M (
    output logic a,
    input  logic b,
    input  logic c = 1'b0,
    input  logic d = 1'b1
);
    assign a = b ^ c ^ d;
endmodule

module test;

  logic a, b, c;
  logic x, y;

  assign b = 1'b0;
  assign c = 1'b1;

  assign y = 1'b1;

  M i_M1 (a, b, c);
  M i_M2 (x, y);

  initial begin
    #1
    if (a !== 1'b0 || x !== 1'b0) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
