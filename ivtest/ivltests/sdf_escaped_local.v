`timescale 1ps/1ps
module local_cell(input \A.dot , output \Z\out );
  buf (\Z\out , \A.dot );
  specify (\A.dot => \Z\out ) = (1, 1); endspecify
endmodule
module top;
  reg A = 0;
  wire [3:0] Z;
  integer count = 0;
  local_cell \back\slash (A, Z[0]);
  local_cell \literal/slash (A, Z[1]);
  local_cell \CELL (A, Z[2]);
  local_cell plain(A, Z[3]);
  initial begin
    $sdf_annotate("ivltests/sdf_escaped_local.sdf", top);
    #100 A = 1;
    #100 A = 0;
    #50;
    if (count != 8) $fatal(1, "FAIL observed %0d events, expected 8", count);
    $display("PASSED");
    $finish;
  end
  genvar i;
  for (i = 0; i < 4; i = i+1) begin: check_output
    integer events = 0;
    always @(Z[i]) if ($time > 50) begin
      events = events + 1;
      count = count + 1;
      if (!((Z[i] === 1'b1 && $time == 103+2*i) ||
            (Z[i] === 1'b0 && $time == 205+2*i)))
        $fatal(1, "FAIL local output %0d at %0t = %b", i, $time, Z[i]);
    end
    final if (events != 2) $fatal(1, "FAIL output %0d observed %0d", i, events);
  end
endmodule
