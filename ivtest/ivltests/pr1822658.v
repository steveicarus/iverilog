`define _variable 1

module top;
  initial begin
    if (`_variable == 1) $display("PASSED");
    else $display("Fail");
  end
endmodule
