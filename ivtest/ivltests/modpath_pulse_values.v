// Preserve separated four-state values, zero delays and vector path behavior.
`timescale 1ps/1ps

module value_path(input i, output o);
  assign o = i;
  specify
    (i => o) = (7, 11, 13);
  endspecify
endmodule

module zero_path(input i, output o);
  assign o = i;
  specify
    (i => o) = (0, 0);
  endspecify
endmodule

module unknown_path(input i, output o);
  assign o = i;
  specify
    (i => o) = 5;
  endspecify
endmodule

module vector_path(input [1:0] i, output [1:0] o);
  assign o = i;
  specify
    (i => o) = (3, 3, 3);
  endspecify
endmodule

module test;
  reg i, zero_i, unknown_i;
  reg [1:0] vector_i;
  wire o, zero_o, unknown_o;
  wire [1:0] vector_o;
  integer changes = 0;
  integer zero_changes = 0;
  integer unknown_changes = 0;
  integer vector_changes = 0;
  reg failed = 0;
  value_path dut(i, o);
  zero_path zero_dut(zero_i, zero_o);
  unknown_path unknown_dut(unknown_i, unknown_o);
  vector_path vector_dut(vector_i, vector_o);

  task check;
    input time expected_time;
    input expected_value;
    begin
      if ($time != expected_time || o !== expected_value) begin
        $display("four-state change %0d: got %0t/%b, expected %0t/%b",
                 changes, $time, o, expected_time, expected_value);
        failed = 1;
      end
    end
  endtask
  always @(o) if ($time >= 50) begin
    case (changes)
      0: check(107, 1'b1);
      1: check(131, 1'bx);
      2: check(153, 1'bz);
      3: check(167, 1'b1);
      4: check(191, 1'b0);
      5: check(207, 1'bx);
      6: check(227, 1'b1);
      7: check(253, 1'bz);
      8: check(271, 1'b0);
      default: begin
        $display("extra four-state change %0d at %0t: %b", changes, $time, o);
        failed = 1;
      end
    endcase
    changes = changes + 1;
  end

  always @(zero_o) if ($time >= 50) begin
    if ((zero_changes == 0 && ($time != 100 || zero_o !== 1'b1)) ||
        (zero_changes == 1 && ($time != 101 || zero_o !== 1'b0)) ||
        (zero_changes == 2 && ($time != 102 || zero_o !== 1'b1)) ||
        zero_changes > 2) begin
      $display("zero-delay change %0d at %0t: %b", zero_changes, $time, zero_o);
      failed = 1;
    end
    zero_changes = zero_changes + 1;
  end

  // A returning X pulse is rejected before a distinct request is rearmed.
  // Nominal X@105/0@107 have width 2 < 5; the persistent 1 is due at 109.
  always @(unknown_o) if ($time >= 50) begin
    if (unknown_changes != 0 || $time != 109 || unknown_o !== 1'b1) begin
      $display("returning-X change %0d: got %0t/%b, expected change 0 at 109/1",
               unknown_changes, $time, unknown_o);
      failed = 1;
    end
    unknown_changes = unknown_changes + 1;
  end

  task check_vector;
    input time expected_time;
    input [1:0] expected_value;
    begin
      if ($time != expected_time || vector_o !== expected_value) begin
        $display("vector change %0d: got %0t/%b, expected %0t/%b",
                 vector_changes, $time, vector_o, expected_time, expected_value);
        failed = 1;
      end
    end
  endtask
  always @(vector_o) if ($time >= 50) begin
    case (vector_changes)
      0: check_vector(103, 2'b11);
      1: check_vector(123, 2'b01);
      2: check_vector(143, 2'b10);
      3: check_vector(163, 2'bxx);
      4: check_vector(183, 2'bzz);
      5: check_vector(203, 2'b00);
      default: begin
        $display("extra vector change %0d at %0t: %b", vector_changes, $time, vector_o);
        failed = 1;
      end
    endcase
    vector_changes = vector_changes + 1;
  end

  initial begin
    i = 0;
    #100 i = 1;
    #20 i = 1'bx;
    #20 i = 1'bz;
    #20 i = 1;
    #20 i = 0;
    #20 i = 1'bx;
    #20 i = 1;
    #20 i = 1'bz;
    #20 i = 0;
  end
  initial begin
    zero_i = 0;
    #100 zero_i = 1;
    #1 zero_i = 0;
    #1 zero_i = 1;
  end
  initial begin
    unknown_i = 0;
    #100 unknown_i = 1'bx;
    #2 unknown_i = 0;
    #2 unknown_i = 1;
  end
  initial begin
    vector_i = 2'b00;
    #100 vector_i = 2'b11;
    #20 vector_i = 2'b01;
    #20 vector_i = 2'b10;
    #20 vector_i = 2'bxx;
    #20 vector_i = 2'bzz;
    #20 vector_i = 2'b00;
  end
  initial begin
    #50;
    if (o !== 1'b0 || zero_o !== 1'b0 || unknown_o !== 1'b0 ||
        vector_o !== 2'b00)
      $fatal(1, "initial outputs did not settle: scalar %b, zero %b, returning-X %b, vector %b",
             o, zero_o, unknown_o, vector_o);
    #250;
    if (failed || changes != 9 || zero_changes != 3 || unknown_changes != 1 ||
        vector_changes != 6 || o !== 1'b0 || zero_o !== 1'b1 ||
        unknown_o !== 1'b1 || vector_o !== 2'b00)
      $fatal(1, "history mismatch: scalar %0d/%b, zero %0d/%b, returning-X %0d/%b, vector %0d/%b",
             changes, o, zero_changes, zero_o, unknown_changes, unknown_o,
             vector_changes, vector_o);
    $display("PASSED");
    $finish(0);
  end
endmodule
