`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg [2:0] res [0:7];
  reg [2:0] in [0:7];
  reg [7:0] dummy [0:6];
  time run_time [0:7];
  time exp_time [0:7];
  integer i;

  initial begin
    pass = 1'b1;
    #1;
    // Initialize the input array.
    for (i=0; i<8; i=i+1) begin
      in[i] = i[2:0];
    end
    #1;
    for (i=0; i<8; i=i+1) begin
      exp_time[i] = $time-1;
    end
    check;

    // We only have 6 dummy items, check that each triggers correctly.
    for (i=0; i<7; i=i+1) begin
      dummy[i] = 1'b0;
      #1;
      exp_time[i] = $time-1;
      check;
    end

    if (pass) $display("PASSED");
  end

  // Check that the value and time are correct.
  task check;
    integer j;
    begin
      for (j=0; j<8; j=j+1) begin
        if (res[j] !== j[2:0]) begin
          $display("FAILED: index %0d value, at %2t, expexted %b, got %b.",
                   j, $time, j[2:0], res[j]);
          pass = 1'b0;
        end
        if (run_time[j] !== exp_time[j]) begin
          $display("FAILED: index %0d time, at %2t, expexted %2t, got %2t.",
                   j, $time, exp_time[j], run_time[j]);
          pass = 1'b0;
        end
      end
    end
  endtask

  genvar m;
  generate
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
     for (m=0; m<=7; m=m+1) begin: idac_loop
`else
     for (m=0; m<=6; m=m+1) begin: idac_loop
`endif
      // This should complain that dummy[7] is out of bounds.
      always @ (in[m] or dummy[m]) begin
        res[m] = in[m];
        run_time[m] = $time;
      end
    end
  endgenerate
endmodule
