`timescale 1ps/1ps
module escaped_cond_cell(input A, input \gate.dot , input \gate/slash ,
                         input \gate"quote , input \gate\back ,
                         output ZD, ZS, ZQ, ZB);
  buf (ZD, A), (ZS, A), (ZQ, A), (ZB, A);
  specify
    if (\gate.dot )   (A => ZD) = (1, 1);
    if (\gate/slash ) (A => ZS) = (1, 1);
    if (\gate"quote ) (A => ZQ) = (1, 1);
    if (\gate\back )  (A => ZB) = (1, 1);
  endspecify
endmodule
module top;
  reg A = 0, E = 0;
  wire [3:0] Z;
  escaped_cond_cell u(A, E, E, E, E, Z[0], Z[1], Z[2], Z[3]);
  initial begin
`ifdef SLASH_DIVIDER
    $sdf_annotate("ivltests/sdf_cond_escaped_slash.sdf", top);
`else
    $sdf_annotate("ivltests/sdf_cond_escaped.sdf", top);
`endif
    #10 E = 1;
    #90 A = 1;
    #100 A = 0;
    #50;
    $display("PASSED");
    $finish(0);
  end
  genvar i;
  for (i = 0; i < 4; i = i+1) begin: check_output
    integer events = 0;
    always @(Z[i]) if ($time > 50) begin
      events = events + 1;
      if (!((events == 1 && Z[i] === 1'b1 && $time == 103+4*i) ||
            (events == 2 && Z[i] === 1'b0 && $time == 205+4*i)))
        $fatal(1, "FAIL escaped condition %0d at %0t = %b", i, $time, Z[i]);
    end
    final if (events != 2) $fatal(1, "FAIL output %0d observed %0d", i, events);
  end
endmodule
