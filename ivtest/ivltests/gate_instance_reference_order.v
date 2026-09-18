// Check that an outer name remains visible before a gate instance declaration.

module test;
  parameter i_g = 23;

  if (1) begin : g
    wire out;

    initial begin
      #1;
      if (i_g === 23 && out === 1'b1) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    end

    and i_g(out, 1'b1, 1'b1);
  end
endmodule
