module top;
  reg result;

  initial begin

    result = $isunknown(top);
    result = $isunknown("a string");
    result = $isunknown(4'b001, 1'b0);

  end

endmodule
