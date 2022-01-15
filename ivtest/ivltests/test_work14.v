module test14;
import work14_pkg::*;
bit clk = 0;

parameter longint maxvalue = 2**29 + 17;
logic [29:0] mvalue;
logic [29:0] lvalue;

initial begin : clkgen forever #10 clk = ~clk; end

assign lvalue = maxvalue;

work14_comp #(.max_out_val(maxvalue)) duv (.clk_i(clk), .val(mvalue));

initial begin
  @(posedge clk);
  #1;
  if (lvalue !== mvalue) $display ("ERROR due to mismatch between lvalue=%d and mvalue=%d", lvalue, mvalue);
  @(posedge clk);
  #1;
  if (lvalue !== mvalue) $display ("ERROR due to mismatch between lvalue=%d and mvalue=%d", lvalue, mvalue);
  #5;
  $display ("PASSED");
  $finish;
end


endmodule
