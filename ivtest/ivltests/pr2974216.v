// Verify that a zero width constant replication is handled correctly.
module top;
  reg pass;
  reg [31:0] in_full;
  wire [31:0] pa_out_full, ca_out_full;
  reg [29:0] in_part;
  wire [31:0] pa_out_part, ca_out_part;

  initial begin
    pass = 1'b1;
    in_full = {16{2'b10}};
    in_part = {15{2'b01}};
    #1;
    if (pa_out_full !== 32'b10101010101010101010101010101010) begin
      $display("Failed: pa_out_full, got %b", pa_out_full);
      pass = 1'b1;
    end
    if (ca_out_full !== 32'b10101010101010101010101010101010) begin
      $display("Failed: ca_out_full, got %b", ca_out_full);
      pass = 1'b1;
    end
    if (pa_out_part !== 32'bxx010101010101010101010101010101) begin
      $display("Failed: pa_out_part, got %b", pa_out_part);
      pass = 1'b1;
    end
    if (ca_out_part !== 32'bzz010101010101010101010101010101) begin
      $display("Failed: ca_out_part, got %b", ca_out_part);
      pass = 1'b1;
    end

    if (pass) $display("PASSED");
  end

  param #(32) full(pa_out_full, ca_out_full, in_full);
  param #(30) part(pa_out_part, ca_out_part, in_part);
endmodule

module param #(parameter width = 32) (
  output reg [31:0] pa_out,
  output wire [31:0] ca_out,
  input [width-1:0] in);

  assign ca_out = {{32-width{1'bz}}, in};

  always @* pa_out = {{32-width{1'bx}}, in};
endmodule
