module add32(sum, cOut, clock, a, b, cIn);

  input clock;
  input a, b, cIn;
  output sum, cOut;

  reg [31:0] a, b;
  reg cIn;
  wire [31:0] sum;
  wire cOut;

  always @(posedge clock)
    //{cOut, sum} = a + b + cIn;
    assign sum = a + b + cIn;

endmodule

//////////////////////////

module main;

    reg CLOCK;
    reg [31:0] A, B;
    reg C_IN;
    reg [31:0] SUM;
    wire C_OUT;


    add32 myAdder(SUM, C_OUT, CLOCK, A, B, C_OUT);

    always #1 CLOCK = ~ CLOCK;

    initial
    begin
      $monitor($time,, " CLOCK=%d, A=%x, B=%x, C_IN=%d -- SUM=%x, C_OUT=%d",
CLOCK, A, B, C_IN, SUM, C_OUT);
    end

    initial
    begin
      CLOCK = 0;
      A = 32'h00000001;
      B = 32'h00000002;
      C_IN = 1'b0;
      #20 $finish;
    end

endmodule
