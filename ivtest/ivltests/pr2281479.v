module top;
  reg passed;
  reg[1:0] in;
  integer where;

  always @(in) begin
    casez(in)
      2'b10: where = 1;
      2'bx?: where = 2; // MSB is X
      2'b??: where = 3; // The same as default.
    endcase
  end

  initial begin
    passed = 1'b1;

    in = 2'b10;
    #1 if (where != 1) begin
      $display("FAILED 2'b10 case, found case %d", where);
      passed = 1'b0;
    end

    in = 2'bx0;
    #1 if (where != 2) begin
      $display("FAILED 2'bx? case (1), found case %d", where);
      passed = 1'b0;
    end
    in = 2'bx1;
    #1 if (where != 2) begin
      $display("FAILED 2'bx? case (2), found case %d", where);
      passed = 1'b0;
    end

    in = 2'b00;
    #1 if (where != 3) begin
      $display("FAILED 2'b?? case, found case %d", where);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
