`timescale 1ns/1ps

module top;
  reg pass;
  reg ina, inb;
  wire out;

  my_or dut(out, ina, inb);

  initial begin
    pass = 1'b1;
    ina = 1'b0;
    inb = 1'b0;
    #0.399
    if (out !== 1'bx && out !== 1'bz) begin
      $display("FAILED: gate had incorrect delay, expected x/z, got %b.", out);
      pass = 1'b0;
    end
    #0.002
    if (out !== 1'b0) begin
      $display("FAILED: gate had incorrect delay, expected 0, got %b.", out);
      pass = 1'b0;
    end

    // Check inertial delays.
    ina = 1'b1;
    #0.399
    ina = 1'b0;
    #0.002
    if (out !== 1'b0) begin
      $display("FAILED: inertial delay, expected 0, got %b.", out);
      pass = 1'b0;
    end

    // Check that this change is relative to the first edge.
    ina = 1'b1;
    #0.200;
    inb = 1'b1;
    #0.201;
    if (out !== 1'b1) begin
      $display("FAILED: double edge delay, expected 1, got %b.", out);
      pass = 1'b0;
      #0.200;
      if (out === 1'b1) begin
        $display("FAILED: double edge delay was off second edge.");
      end
    end

    if (pass) $display("PASSED");
  end
endmodule

module my_or(out, ina, inb);
  output out;
  input ina, inb;

  or(out, ina, inb);

  specify
    (ina, inb *> out) = 0.4;
  endspecify
endmodule
