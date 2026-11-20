// Check that an event control on a constant bit or part select of a
// vector detects every change of the selected bits, also when they
// change and change back within one time step after the process was
// woken by another item of its event list.
//
// At each rising clock edge va and size get new values from nonblocking
// assignments, and add passes through an intermediate value on its way
// to the new one. When va goes from 8'hfc to 8'h48 and size from 1 to 7,
// add[3:2] goes 11 -> 10 -> 11 while va[3:2] goes 11 -> 10. A process
// that runs on the va[3:2] change while add[3:2] is 10 must run again
// when add[3:2] returns to 11.

module test;

  reg        clk = 1'b0;
  reg  [7:0] va;
  reg  [3:0] size;
  wire [4:0] add = {1'b0, va[3:0]} + {1'b0, size};

  reg  [3:0] en_part, en_bits, en_index;
  reg  [1:0] pulse = 2'b00;
  integer    cycles = 0, posedges = 0;
  reg        failed = 1'b0;

  always @(add[3:2] or va[3:2])
    en_part = {va[3:2], add[3:2]};

  always @(add[3] or add[2] or va[3] or va[2])
    en_bits = {va[3:2], add[3:2]};

  always @(add[2 +: 2] or va[3 -: 2])
    en_index = {va[3:2], add[3:2]};

  always @(posedge pulse[0])
    posedges = posedges + 1;

  always @(posedge clk) begin
    if (cycles % 2 == 0) begin
      va <= 8'h48;
      size <= 4'd7;
    end else begin
      va <= 8'hfc;
      size <= 4'd1;
    end
    cycles = cycles + 1;
  end

  `define check(name, val) \
    if (val !== {va[3:2], add[3:2]}) begin \
      $display("FAILED: %s is %b after cycle %0d, expected %b", name, \
               val, cycles, {va[3:2], add[3:2]}); \
      failed = 1'b1; \
    end

  always @(negedge clk) begin
    if (cycles > 1) begin
      `check("en_part", en_part);
      `check("en_bits", en_bits);
      `check("en_index", en_index);
    end
  end

  initial begin
    repeat (8) begin
      #5 clk = 1'b1;
      #5 clk = 1'b0;
    end
    // A pulse of zero width on bit 0 is a positive edge of pulse[0].
    pulse = 2'b01;
    pulse = 2'b00;
    #1;
    if (posedges !== 1) begin
      $display("FAILED: %0d positive edges of pulse[0], expected 1",
               posedges);
      failed = 1'b1;
    end
    if (!failed)
      $display("PASSED");
    $finish;
  end

endmodule
