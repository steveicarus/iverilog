module top;
  real rval;
  integer res;
  reg [7:0] rg;
  reg [7:0] mem [3:0];

  initial begin
    res = $fread(rval, 1); // 1st arg. must be a reg. or memory.
    res = $fread(rg); // Too few argument.
    res = $fread(mem, "a"); // Not a valid fd.
    res = $fread(mem, 1, "a"); // Not a valid start.
    res = $fread(mem, 1, 0, "a"); // Not a valid count.
    res = $fread(mem, 1, 0, 2, 3); // Too many argument.
  end
endmodule
