module top;
  parameter amax = 9;
  reg [31:0] mem [amax:0];
  integer i, tmp;
  integer fail [amax:0];
  integer pass;

  initial begin
    pass = 1;
    for (i=0; i<amax; i=i+1) begin
      fail[i] = i;
      mem[i] = i;
    end
    for (i=0; i<(amax-1); i=i+1) begin
      mem[fail[i]] = mem[fail[i]] + 1;
//      tmp = fail[i];
//      mem[tmp] = mem[tmp] + 1;
    end
    for (i=0; i<(amax-1); i=i+1) begin
      if (mem[i] != i+1) begin
        pass = 0;
        $display("Failed location %d, value was %d, expected %d",
                 i, mem[i], i+1);
      end
    end
    if (pass) $display("PASSED");
  end
endmodule
