module top;
  integer res;

  real rvar [1:0];
  realtime rtvar [1:0];

  wire real rnet [1:0];

  assign rnet[0] = 2.0;

  initial begin
    rvar[0] = -1.0;
    rtvar[0] = 1.0;
    #1;
    // Check the various get routines.
    $display("Real %g, Realtime %g", rvar[0], rtvar[0]);
    $display("Real as int %d, Realtime as int %d", rvar[0], rtvar[0]);
    $display("Real net %g", rnet[0]);
    $display("Real net as int %d", rnet[0]);

    // Check some put routines.
    res = $sscanf("3.5", "%f", rvar[1]);
    if (rvar[1] != 3.5) $display("Failed %%f put");
    else $display("Passed %%f put");

    res = $sscanf("4", "%d", rtvar[1]);
    if (rtvar[1] != 4.0) $display("Failed %%d put");
    else $display("Passed %%d put");
  end
endmodule
