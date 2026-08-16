// Check that enclosing instance names do not affect compilation-unit variable
// lookup for implicit and wildcard port connections.

reg [31:0] named_value = 41;
reg [31:0] wildcard_value = 43;

module holder;
endmodule

module port_target(
    input wire [31:0] named_value,
    input wire [31:0] wildcard_value,
    output wire passed
);

  assign passed = named_value == 32'd41 && wildcard_value == 32'd43;

endmodule

module child;

  wire passed;

  port_target target(.named_value, .passed(passed), .*);

  initial begin
    #1;

    if (passed !== 1) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder named_value();
  holder wildcard_value();
  child child_instance();

endmodule
