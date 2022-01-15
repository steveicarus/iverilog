`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module t();
  reg passed;
  parameter ch = 14;
  parameter csek2 = 1;
  parameter offset = 10/0;
  localparam csek = 1 << csek2;

  wire [ch + csek2 - 1:0] cim_k;
  wire [csek - 1:0] up1, up2, up3, up4, up5, dwn1, dwn2, dwn3;

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  // This checks the always above code.
  assign up1 = cim_k[(csek2 + ch)+:csek2];
  // This checks the always above code.
  assign up2 = cim_k[(csek2 + ch + 2)+:csek2];
  // This checks the always below code.
  assign up3 = cim_k[(csek2 + ch - 17)+:csek2];
  // This checks that -4 goes into three bits not two.
  assign up4 = cim_k[(csek2 + ch - 18)+:csek2];
  // This checks that an undef base gives 'bx out.
  assign up5 = cim_k[(csek2 + ch - offset)+:csek2];

  // This checks the always above code.
  assign dwn1 = cim_k[(csek2 + ch + 2)-:csek2];
  // This checks the always below code.
  assign dwn2 = cim_k[(csek2 + ch - 17)-:csek2];
  // This checks that an undef base gives 'bx out.
  assign dwn3 = cim_k[(csek2 + ch - offset)-:csek2];
`else
  assign up1 = 2'b0x;
  assign up2 = 2'b0x;
  assign up3 = 2'b0x;
  assign up4 = 2'b0x;
  assign up5 = 2'b0x;
  assign dwn1 = 2'b0x;
  assign dwn2 = 2'b0x;
  assign dwn3 = 2'b0x;
`endif

  initial begin
    #1;
    passed = 1'b1;

    if (cim_k !== 15'bz) begin
      $display("FAILED: cim_k should be 15'bz, got %b", cim_k);
      passed = 1'b0;
    end

    if (up1 !== 2'b0x) begin
      $display("FAILED: up1 should be 2'b0x, got %b", up1);
      passed = 1'b0;
    end

    if (up2 !== 2'b0x) begin
      $display("FAILED: up2 should be 2'b0x, got %b", up2);
      passed = 1'b0;
    end

    if (up3 !== 2'b0x) begin
      $display("FAILED: up3 should be 2'b0x, got %b", up3);
      passed = 1'b0;
    end

    if (up4 !== 2'b0x) begin
      $display("FAILED: up4 should be 2'b0x, got %b", up4);
      passed = 1'b0;
    end

    if (up5 !== 2'b0x) begin
      $display("FAILED: up5 should be 2'b0x, got %b", up5);
      passed = 1'b0;
    end

    if (dwn1 !== 2'b0x) begin
      $display("FAILED: dwn1 should be 2'b0x, got %b", dwn1);
      passed = 1'b0;
    end

    if (dwn2 !== 2'b0x) begin
      $display("FAILED: dwn2 should be 2'b0x, got %b", dwn2);
      passed = 1'b0;
    end

    if (dwn3 !== 2'b0x) begin
      $display("FAILED: dwn3 should be 2'b0x, got %b", dwn3);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
