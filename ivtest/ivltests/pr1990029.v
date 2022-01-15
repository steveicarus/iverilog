`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top();
  reg pass = 1'b1;
  integer fp, i, n;
  reg [8:0]  v;
`ifndef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  reg [11:-4] w;
`endif

  initial begin
    fp = $fopen("work/temp.txt", "w");
    for (i = 0; i < 4; i = i + 1) begin
      $fdisplay(fp, "%d", i + 16'he020);
    end
    $fclose(fp);

    fp = $fopen("work/temp.txt", "r");
    for (i = 0; i < 4; i = i + 1) begin
      v = 9'd0;
      // Use the following line and change the base in the a.out file
      // to -4  and the width to 16 to get correct functionality.
      //n = $fscanf(fp, " %d ", v[7:0]);  // This uses the &PV<>
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
      n = $fscanf(fp, " %d ", v[11:-4]);  // This does not use &PV<> (bug)
`else
      n = $fscanf(fp, " %d ", w);
      v = w[8:0];
`endif
      if (v != 2) begin
        $display("FAILED: iteration %d, got %b, expected 9'b000000010", i, v);
        pass = 1'b0;
      end
    end
    $fclose(fp);

    if (pass) $display("PASSED");
  end
endmodule
