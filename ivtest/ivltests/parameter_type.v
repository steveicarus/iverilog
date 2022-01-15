module top;
  parameter irparam = -1.0;
  parameter iiparam = -1;
  parameter [7:0] uparam = -1.0;
  parameter signed [7:0] sparam = -1.0;
  parameter real rparam = -1;
  parameter realtime rtparam = -1;
  parameter integer iparam = -1.0;
  parameter time tparam = -1.0;

  initial begin
    $display("Implicit real: ", irparam);
    $display("Implicit integer: ", iiparam);
    $display("Unsigned: ", uparam);
    $display("Signed: ", sparam);
    $display("Real: ", rparam);
    $display("Real time: ", rtparam);
    $display("Integer: ", iparam);
    $display("Time: ", tparam);
  end
endmodule
