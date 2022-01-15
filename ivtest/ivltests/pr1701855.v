`timescale 1us/1ns

module top;
  initial begin

    // This should print the following:

    // Time scale of (top) is 1us / 1ns
    // Time scale of (top) is 1us / 1ns
    // Time scale of (top.dut) is 10ns / 10ps
    // Time scale of (top.dut.dut) is 1ns / 10ps
    // Time scale of (othertop) is 1ms / 1us

    // But currently the precisions will all be 10ps the finest precision.
    $printtimescale;
    $printtimescale();
    $printtimescale(dut);
    $printtimescale(dut.dut);
    $printtimescale(othertop);
  end

  lower dut();
endmodule

`timescale 10ns/10ps
module lower;
  evenlower dut();
endmodule

`timescale 1ns/10ps
module evenlower;
endmodule

`timescale 1ms/1us
module othertop;
endmodule
