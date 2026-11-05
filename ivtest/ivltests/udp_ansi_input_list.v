// Check that an ANSI UDP input declaration can contain multiple port names.

primitive p(output o, input a, b, input c, d);
  table
    0 1 0 1 : 1;
  endtable
endprimitive

module test;

  wire o;

  p i_p(o, 1'b0, 1'b1, 1'b0, 1'b1);

  initial begin
    #1;
    if (o !== 1'b1) begin
      $display("FAILED: o=%b", o);
    end else begin
      $display("PASSED");
    end
  end

endmodule
