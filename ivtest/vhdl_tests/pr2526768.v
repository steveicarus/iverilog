module top();
  wire [7:0] Z, CO;
  reg  [7:0] A, B, CI;
  reg  ok;

  test test(Z, CO, A, B, CI);

  task check;
    input [7:0] want_z, want_co;
    begin
      $display("A = %d, B = %d, CI = %d", A, B, CI);
      $display("==> Z = %d, CO = %d", Z, CO);
      $display("??? Z = %d, CO = %d", want_z, want_co);
      if (want_co !== CO || want_z !== Z) begin
        $display("FAILED");
        ok = 0;
      end
      $display;
    end
  endtask // check


  initial begin
    ok = 1;
    A = 0;
    B = 0;
    CI = 0;
    #1 check(0, 0);

    A = 4;
    #1 check(4, 0);

    B = 251;
    #1 check(255, 0);

    if (ok)
      $display("PASSED");
  end

endmodule // top

module test (Z, CO, A, B, CI);
  parameter width=8;

  input  [width-1:0] A, B, CI;
  output [width-1:0] Z, CO;

  reg [width-1:0] Z, CO;

  integer i;

  always @(A or B or CI) begin
    for(i=0; i < width; i=i+1)
      {CO[i],Z[i]} = A[i] + B[i] + CI[i];
  end

endmodule
