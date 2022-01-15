module top;
  reg pass = 1'b1;
  reg signed [7:0] arr [0:7];
  integer idx;
  reg signed [1:0] idx2;
  reg [7:0] res;

  real rarr [0:7];
  real rres;

  reg signed [2:0] a [0:3];
  wire signed [2:0] n [0:3];
  assign n[3] = 0;
  assign n[0] = -1;

  initial for (idx = 0; idx < 8 ; idx = idx + 1) rarr[idx] = idx+1.0;
  initial for (idx = 0; idx < 8 ; idx = idx + 1) arr[idx] = idx+1;
  initial begin
    #1;
    idx = 'bx;

    // Test %load/ar
    rres = rarr[idx];
    if (rres != 0.0) begin
      $display("Failed simple real array test, got %f.", rres);
      pass = 1'b0;
    end

    // Test %load/av
    res = arr[idx];
    if (res !== 8'bx) begin
      $display("Failed simple array vector test, got %b.", res);
      pass = 1'b0;
    end

    // Test %load/avp0
    res = arr[idx]+8'b1;
    if (res !== 8'bx) begin
      $display("Failed array vector plus constant test, got %b.", res);
      pass = 1'b0;
    end

    // Test %load/avp0/s
    res = arr[idx]+8'sb1;
    if (res !== 8'bx) begin
      $display("Failed array vector plus signed constant test, got %b.", res);
      pass = 1'b0;
    end

    // Test %load/avx.p with a 'bx array select.
    res = arr[idx][0];
    if (res !== 8'b0x) begin
      $display("Failed array vector bit select test 1, got %b.", res);
      pass = 1'b0;
    end
    // Test %load/avx.p with a 'bx bit select.
    res = arr[0][idx];
    if (res !== 8'b0x) begin
      $display("Failed array vector bit select test 2, got %b.", res);
      pass = 1'b0;
    end
    // Test %load/avx.p with a negative bit select.
    idx2 = -1;
    res = arr[0][idx2];
    if (res !== 8'b0x) begin
      $display("Failed array vector bit select test 3, got %b.", res);
      pass = 1'b0;
    end

    // This should be out of bounds and should be 'bx, but if this
    // is being loaded as an unsigned value it will return 1.
    idx2 = -1;
    a[3] = 0;
    a[0] = -1;
    res = arr[a[idx2]];
    if (res !== 8'bx) begin
      $display("Failed: reg array select driving array select, got %b", res);
      pass = 1'b0;
    end

    // The same here, but check a net array.
    res = arr[n[idx2]];
    if (res !== 8'bx) begin
      $display("Failed: wire array select driving array select, got %b", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
