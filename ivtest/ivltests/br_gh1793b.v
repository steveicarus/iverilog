// Check output arguments of an automatic task whose actual is a bit select,
// part select, or word with a non-constant index in the calling automatic task.
// Both the plain variable and the expression forms of the index are used.

module test;

  reg [7:0] mem [0:7];
  reg [31:0] vec;
  reg [3:0] bits;
  reg failed;

  task automatic put8(input [7:0] a, output [7:0] v);
    v = a;
  endtask

  task automatic put1(input a, output v);
    v = a;
  endtask

  task automatic run(input integer at, input integer hi);
    put8(8'h21, mem[at*2 - 1]);
    put8(8'h22, vec[at*8 +: 8]);
    put8(8'h23, vec[hi -: 8]);
    put1(1'b1, bits[at]);
  endtask

  initial begin
    failed = 1'b0;
    vec = 32'h0;
    bits = 4'b0;
    run(1, 31);
    if (mem[1] !== 8'h21) begin
      $display("FAILED: mem[1] is %h", mem[1]);
      failed = 1'b1;
    end
    if (vec !== 32'h2300_2200) begin
      $display("FAILED: vec is %h", vec);
      failed = 1'b1;
    end
    if (bits !== 4'b0010) begin
      $display("FAILED: bits is %b", bits);
      failed = 1'b1;
    end
    if (!failed) $display("PASSED");
  end

endmodule
