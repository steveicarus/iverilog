// Check that a driver on a later input port member propagates out.

module M(.p({a, b}));

  input a, b;

  assign b = 1'b1;

endmodule

module test;

  wire [1:0] p;

  M i_m(.p(p));

  initial begin
    #1;
    if (p === 2'bz1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
