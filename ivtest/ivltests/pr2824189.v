`begin_keywords "1364-2005"
module top;
  reg pass;
  reg [3:0] var;
  integer fd, code;

  initial begin
    pass = 1'b1;
    fd = $fopen("ivltests/pr2824189.txt", "r");
    code = $fscanf(fd, "%x\n", var);
    if (code != 1) begin
      $display("Failed initial variable read count expected 1, got %d", code);
      pass = 1'b0;
    end
    if (var !== 4'ha) begin
      $display("Failed initial variable read value expected a, got %h", var);
      pass = 1'b0;
    end

    code = $fscanf(fd, "%x\n", var);
    if (code != -1) begin
      $display("Failed $fscanf() at EOF");
      pass = 1'b0;
    end

    $fclose(fd);

    if (pass)  $display("PASSED");
  end
endmodule
`end_keywords
