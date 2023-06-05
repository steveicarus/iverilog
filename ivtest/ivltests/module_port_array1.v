// Check that connecting a module port array with a single element is supported

module M (
  input [7:0] in[0:0],
  output [7:0] out[0:0]
);

  assign out[0] = in[0];

endmodule

module test;

  reg [7:0] A[0:0];
  wire [7:0] B[0:0];

  M i_m (
    .in(A),
    .out(B)
  );

  initial begin
    A[0] = 10;
    #1
    if (B[0] === 10) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
