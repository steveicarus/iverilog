// Check that unconnected_drive pulls every input member in every instance.

`unconnected_drive pull0

module M(.p({o, a, b}));

  output o;
  input a, b;

endmodule

`nounconnected_drive

module test;

  M i_m[1:0](.p());

  initial begin
    #1;
    if ({i_m[0].o, i_m[0].a, i_m[0].b,
         i_m[1].o, i_m[1].a, i_m[1].b} === 6'bz00z00) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
