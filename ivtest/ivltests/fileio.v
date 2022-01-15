`begin_keywords "1364-2005"
module top;
  reg [20*8-1:0] var;
  integer fp, code;
  initial begin
    fp = $fopenr("read");
    if (fp != 0) $display("Read of empty file failed.");

    fp = $fopenw("work/test.txt");
    $fdisplay(fp, "From the write.");
    $fclose(fp);

    fp = $fopena("work/test.txt");
    $fdisplay(fp, "From the append.");
    $fclose(fp);

    fp = $fopenr("work/test.txt");
    code = $fgets(var, fp);
    $display("%0s", var[20*8-1:8]);
    code = $fgets(var, fp);
    $display("%0s", var[20*8-1:8]);
    $fclose(fp);
  end
endmodule
`end_keywords
