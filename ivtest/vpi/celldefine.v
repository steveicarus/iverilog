module top;
  reg pass;
  reg in;
  wire out, outb;

  lower_cell dutb(outb, in);
  lower_no_cell dut(out, in);

  always @(outb) begin
    if (outb !== ~in) begin
      $display("FAILED outb at time %t, expected %b, got %b", $time, ~in, outb);
      pass = 1'b0;
    end
  end

  always @(out) begin
    if (out !== in) begin
      $display("FAILED out at time %t, expected %b, got %b", $time, in, out);
      pass = 1'b0;
    end
  end

  initial begin
    pass = 1'b1;
    #1 in = 1'b0;
    #1 in = 1'b1;
    #1 in = 1'b0;

    #1 if (pass) $display("Verilog checking was OK.");
    $is_cell(dut);
    $is_cell(dutb);
  end
endmodule

`celldefine
module lower_cell(output out, input in);
  not(out, in);
endmodule
`endcelldefine

module lower_no_cell(output out, input in);
  buf(out, in);
endmodule
