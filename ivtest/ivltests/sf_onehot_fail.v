module top;
  reg result;

  initial begin

    result = $onehot(top);
    result = $onehot("a string");
    result = $onehot(4'b001, 1'b0);

  end

endmodule
