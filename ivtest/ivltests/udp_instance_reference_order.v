// Check that an outer name remains visible before a UDP instance declaration.

primitive P(out, in);
  output out;
  input in;

  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

module test;
  parameter i_p = 23;

  if (1) begin : g
    wire out;

    initial begin
      #1;
      if (i_p === 23 && out === 1'b1) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    end

    P i_p(out, 1'b1);
  end
endmodule
