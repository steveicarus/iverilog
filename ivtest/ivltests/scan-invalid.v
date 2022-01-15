module top;
  integer res, idx;
  wire [24*8-1:0] wval;
  wire [24*8-1:0] warr [0:1];
  reg [24*8-1:0] rval;

  initial begin
    idx = 0;
//    res = $sscanf(); // A function must have at least one argument!
    res = $sscanf("A string");
    res = $sscanf(wval);
    res = $sscanf("A string", wval);
    res = $sscanf("A string", "%s", top);
    res = $sscanf("A string", "%s", wval);
    res = $sscanf("A string", "%s %s", rval, warr[0]);

    // This is an invalid fd, but it passes the compiletf routine checks.
    // It would get caught by the run time if the compiletf routines did
    // not fail.
    res = $fscanf(1);
    res = $fscanf("A string");
    res = $fscanf(1, wval);
    res = $fscanf(1, "%s", top);
    res = $fscanf(1, "%s", wval);
    res = $fscanf(1, "%s %s", rval, warr[0]);

  end
endmodule
