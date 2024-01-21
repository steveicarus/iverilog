module top;
  reg pass;
  reg [7:0] idx;
  reg [7:0] mem [0:7];

  initial begin
    pass = 1'b1;

    // Neither no_dir or no_dir2 should exist and vsim should be a file.
    // The ivltests directory should exist.
    $readmempath("/no_dir:no_dir2:vsim:ivltests");

    $readmemh("pr2509349.txt", mem);

    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (mem[idx] !== idx) begin
        $display("Failed mem[%0d], expected %d, got %d", idx, idx, mem[idx]);
        pass = 1'b0;
      end
    end

    if (pass) $display("PASSED");
  end
endmodule
