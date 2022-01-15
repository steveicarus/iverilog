module top;
  integer result;

  initial begin

    result = $countones(top);
    result = $countones("a string");
    result = $countones(4'b001, 1'b0);

  end

endmodule
