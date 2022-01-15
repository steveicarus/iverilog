module top;
  reg [256*8:1] name;

  initial begin
    name = "work/dumptest";
    $dumpfile({name, ".vcd"});
    $dumpvars;
  end
endmodule
