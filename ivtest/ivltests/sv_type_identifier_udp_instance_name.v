// Check that a UDP instance can shadow a previously used outer typedef.

typedef logic [7:0] i_p;

primitive P(out, in);
  output out;
  input in;
  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

module test;
  wire out;

  i_p value; // Declares variable using the outer typedef.
  P i_p(out, 1'b1); // Instantiates primitive and shadows the outer typedef.

  initial begin
    value = 8'h2a;
    #1;
    if (out !== 1'b1 || value !== 8'h2a) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end
endmodule
