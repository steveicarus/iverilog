`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass = 1'b1;
  wire [3:0] part_idx_up, part_idx_down, part_sel;
  wire [3:0] ps_array [1:0];

  // Check the positive indexed part select.
//  assign part_idx_up[-1+:2] = 2'b01; // We do not currently support this!
  assign part_idx_up[1+:2] = 2'b01;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign part_idx_up[3+:2] = 2'b01;
  assign part_idx_up[5+:2] = 2'b01;  // This should be skipped
  assign part_idx_up[7+:3] = 3'b001; // This should be skipped
`else
  assign part_idx_up[3] = 1'b1;
`endif

  // Check the negative indexed part select.
//  assign part_idx_down[0-:2] = 2'b10; // We do not currently support this!
  assign part_idx_down[2-:2] = 2'b10;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign part_idx_down[4-:2] = 2'b10;
  assign part_idx_down[6-:2] = 2'b10;  // This should be skipped
  assign part_idx_down[9-:3] = 3'b100; // This should be skipped
`else
  assign part_idx_down[3] = 1'b0;
`endif

  // Check a normal constant part select.
//  assign part_sel[1:-1] = 2'b01; // We do not currently support this!
  assign part_sel[2:1] = 2'b01;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign part_sel[4:3] = 2'b01;
  assign part_sel[6:5] = 2'b01;  // This should be skipped
  assign part_sel[9:7] = 3'b001; // This should be skipped
`else
  assign part_sel[3] = 1'b1;
`endif

  // Check a normal constant part select on an array.
//  assign ps_array[0][1:-1] = 2'b01; // We do not currently support this!
  assign ps_array[0][2:1] = 2'b01;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign ps_array[0][4:3] = 2'b01;
  assign ps_array[0][6:5] = 2'b01;  // This should be skipped
  assign ps_array[0][9:7] = 3'b001; // This should be skipped
`else
  assign ps_array[0][3] = 1'b1;
`endif

  initial begin
    #1;
    if (part_idx_up !== 4'b101z) begin
      $display("Failed +: select, expected 4'b101z, got %b", part_idx_up);
      pass = 1'b0;
    end

    if (part_idx_down !== 4'b010z) begin
      $display("Failed -: select, expected 4'b010z, got %b", part_idx_down);
      pass = 1'b0;
    end

    if (part_sel !== 4'b101z) begin
      $display("Failed const. part select, expected 4'b101z, got %b", part_sel);
      pass = 1'b0;
    end

    if (ps_array[0] !== 4'b101z) begin
      $display("Failed array part select, expected 4'b101z, got %b",
               ps_array[0]);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
