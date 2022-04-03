module test;

localparam NOM_COUNT = 1000;
localparam MIN_COUNT = NOM_COUNT - 200;
localparam MAX_COUNT = NOM_COUNT + 200;

integer histogram[255:0];

integer i;

reg [7:0] value;

reg failed;

initial begin
  failed = 0;
  for (i = 0; i < 256; i++) begin
    histogram[i] = 0;
  end
  for (i = 0; i < 256*NOM_COUNT; i = i + 1) begin
    value = $urandom_range(32'hffffffff, 32'h0) >> 24;
    histogram[value] += 1;
  end
  for (i = 0; i < 256; i++) begin
    if (histogram[i] < MIN_COUNT) begin
      $display("Bin %3d count %0d", i, histogram[i]);
      failed = 1;
    end
    if (histogram[i] > MAX_COUNT) begin
      $display("Bin %3d count %0d", i, histogram[i]);
      failed = 1;
    end
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
