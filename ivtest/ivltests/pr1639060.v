// pr1639060

module top;
  real value;

  initial begin
    value = 10.0;
    // value = -10.0;
    print;
  end

  task print;

    real tmp;

    begin
      if (value < 0.0) tmp = value + 10.0;
      else tmp = value;
      $display("1. The result is %5.1f", tmp);

      // This line fails!
      tmp = (value < 0.0) ? value+10.0 : value;
      $display("2. The result is %5.1f", tmp);
    end
  endtask
endmodule
