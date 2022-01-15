module top;
  reg [4:1] res;
  reg pass;

  initial begin
    pass = 1'b1;
    res = 4'b0000;
    fork
      #3 res[3] = 1'b1;
      #4 res[4] = 1'b1;
    join_none

    fork
      #1 res[1] = 1'b1;
      #2 res[2] = 1'b1;
    join_any

    if (res != 4'b0001) begin
      $display("Error: Only first process should have run: %b", res);
      pass = 1'b0;
    end

    wait fork;
    if (res != 4'b1111) begin
      $display("Error: All processes should have run: %b", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
