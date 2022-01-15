`begin_keywords "1364-2005"
module main;
  reg pass;
  reg a, b;

  always @* begin
    b = a;
    #2;
    b = 1'b0;
  end

  task check_bit;
    input bit;

    begin
      a = bit;
      #1 if (a !== b) begin
        $display("FAILED, expected %b, got %b", a, b);
        pass = 1'b0;
      end
      #2 if (b !== 1'b0) begin
        $display("FAILED return to zero, got %b", b);
        pass = 1'b0;
      end
    end
  endtask

  initial begin
    pass = 1'b1;

    check_bit(1'b1);
    check_bit(1'b0);
    check_bit(1'bx);
    check_bit(1'bz);

    if (pass) $display("PASSED");
  end

endmodule
`end_keywords
