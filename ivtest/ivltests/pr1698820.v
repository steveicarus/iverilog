`begin_keywords "1364-2005"
module top;
  reg [4:0] var;
  integer fp;

  initial begin
    var = 10;
    fp = 1;
    $fdisplay(fp, "The variable is ",var);
    $fdisplayb(fp, "The variable is ",var);
    $fdisplayo(fp, "The variable is ",var);
    $fdisplayh(fp, "The variable is ",var);
    $fwrite(fp, "The variable is ",var, "\n");
    $fwriteb(fp, "The variable is ",var, "\n");
    $fwriteo(fp, "The variable is ",var, "\n");
    $fwriteh(fp, "The variable is ",var, "\n");
    $fclose(fp);
  end
endmodule
`end_keywords
