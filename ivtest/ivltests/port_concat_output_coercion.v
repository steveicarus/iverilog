// Check that a later output port member can receive an external driver.

module M(.p({a, b}));

  output a, b;

  assign a = ~b;

endmodule

module test;

  wire [1:0] p;
  assign p[0] = 1'b0;

  M i_m(.p(p));

  initial begin
    #1;
    if (p === 2'b10) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
