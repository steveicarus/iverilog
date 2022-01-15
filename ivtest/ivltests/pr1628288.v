// `define READ read_good

module top;
  reg clk;
  reg dout;
  reg [7:0] data;
  integer lp;

  always #10 clk = ~clk;  // Build a clock generator.

  always @(negedge clk) dout = ~dout;  // Build a bit stream.

  initial begin
    clk = 0;
    dout = 0;
    @(negedge clk);
    for (lp=0; lp<4; lp=lp+1) begin
      #0 read_bad(data);
      $display("Read(%0d) %b from the bit stream.", lp, data);
      #20;  // For the fun of it skip a clock.
    end

    #1 $finish(0);
  end

  // This one locks up on the third call.
  task read_bad;
    output [7:0] edata;

    integer i;

    reg [7:0] rddata;

    begin
      for(i=7; i>=0; i=i-1) begin
        @(posedge clk);
        $display("  Reading bit %0d", i);
        rddata[i] = dout;  // <<--- This appears to be the problem line!
      end

      assign edata = rddata;
    end
  endtask

  // This one works fine.
  task read_good;
    output [7:0] edata;

   integer i;

    reg [7:0] edata;

    begin
      for(i=7; i>=0; i=i-1) begin
        @(posedge clk);
        $display("  Reading bit %0d", i);
        edata[i] = dout;
      end
    end
  endtask
endmodule
