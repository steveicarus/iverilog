module top;
  // The array code does not currently work because we need &APV<>!
  // Both &PV<> and &APV<> (when implemented) need to have bit
  // specific value change callbacks to function correctly.
  reg [7:0] array [1:0];
  reg [7:0] bs, ps;
  integer idx;

  initial begin
    bs = 8'b0;
    ps = 8'b0;
    array[0] = 8'b0;
    $monitor($time," BS = ", bs[1], ", PS = ", ps[2:1], ", AR = ", array[0][1]);

    // This should only trigger the $monitor when bit 1 changes.
    for (idx = 0; idx < 8 ; idx = idx + 1) begin
      #1 bs[idx] = 1'b1;
    end

    // This should only trigger the $monitor when bit 1 or 2 changes.
    for (idx = 0; idx < 8 ; idx = idx + 1) begin
      #1 ps[idx] = 1'b1;
    end

    // This should only trigger the $monitor when bit 1 of array[0] changes..
    for (idx = 0; idx < 8 ; idx = idx + 1) begin
      #1 array[0][idx] = 1'b1;
    end
  end
endmodule
