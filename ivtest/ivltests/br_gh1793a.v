// Check that an automatic task can pass an array word with a variable index
// as an output argument of a call to another automatic task. The index uses
// variables of the calling task, which must not be read from the context of
// the called task.

module test;

  reg [7:0] buffer [0:15];
  integer got;
  reg failed;

  task automatic read_byte(input [15:0] addr, output [7:0] value);
    value = addr[7:0];
  endtask

  task automatic read_bytes(input integer at, input integer n,
                            output integer count);
    count = 0;
    while (count < n) begin
      read_byte(16'h0040 + count, buffer[at+count]);
      count = count + 1;
    end
  endtask

  task automatic read_one(input integer at);
    read_byte(16'h0055, buffer[at]);
  endtask

  initial begin
    failed = 1'b0;
    read_bytes(2, 4, got);
    if (got !== 4) begin
      $display("FAILED: count is %0d", got);
      failed = 1'b1;
    end
    if (buffer[2] !== 8'h40 || buffer[3] !== 8'h41 ||
        buffer[4] !== 8'h42 || buffer[5] !== 8'h43) begin
      $display("FAILED: buffer[2:5] is %h %h %h %h",
               buffer[2], buffer[3], buffer[4], buffer[5]);
      failed = 1'b1;
    end
    if (buffer[1] !== 8'hxx || buffer[6] !== 8'hxx) begin
      $display("FAILED: unrelated words changed: %h %h", buffer[1], buffer[6]);
      failed = 1'b1;
    end

    read_one(9);
    if (buffer[9] !== 8'h55) begin
      $display("FAILED: buffer[9] is %h", buffer[9]);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end

endmodule
