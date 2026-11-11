// Check that input variables receive values through a wrapper module.

module M(input var logic [7:0] a, output var logic [7:0] y);

  always @* begin
    y = a + 8'd1;
  end

endmodule

module W(input [7:0] a, output [7:0] y);

  M i_m(.a(a), .y(y));

endmodule

module test;

  reg [7:0] a;
  wire [7:0] y;

  W i_w(.a(a), .y(y));

  initial begin
    a = 8'd5;
    #1;
    if (y === 8'd6) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
