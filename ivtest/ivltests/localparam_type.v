module top;
  localparam irparam = -1.0;
  localparam iiparam = -1;
  localparam [7:0] uparam = -1.0;
  localparam signed [7:0] sparam = -1.0;
  localparam real rparam = -1;
  localparam realtime rtparam = -1;
  localparam integer iparam = -1.0;
  localparam time tparam = -1.0;

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
