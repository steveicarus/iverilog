// A one-element vector is dumped as name[msb:lsb] and a scalar as name.
// Print the $var lines of the VCD file that was just written.

module test;

  logic a;
  logic [0:0] b;
  logic [3:3] c;
  logic [7:0] d;
  logic signed [0:0] e;
  bit f;
  bit [0:0] g;
  int h;
  real r;
  wire w = 1'b1;
  wire [0:0] wv = 1'b1;
  wire [3:3] wv3 = 1'b1;

  integer fd;
  string line;

  initial begin
    a = 0; b = 0; c = 0; d = 0; e = 0; f = 0; g = 0; h = 0; r = 0.0;
    $dumpfile("work/vcd_scalar_vector.vcd");
    $dumpvars(0, test);
    #1 a = 1;
    $dumpflush;
    fd = $fopen("work/vcd_scalar_vector.vcd", "r");
    while ($fgets(line, fd) != 0)
      if (line.substr(0, 3) == "$var")
        $write("%s", line);
    $fclose(fd);
  end

endmodule
