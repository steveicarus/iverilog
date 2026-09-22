module top;
  parameter string pin = "Hello World!";
  parameter string pup = pin.toupper();
  parameter string plw = pin.tolower();

  logic passed;
  string in, out;

  initial begin
    passed = 1'b1;

    if (pin != "Hello World!") begin
      $display("Expected `Hello World!`, got `%s`", pin);
      passed = 1'b0;
    end

    if (pup != "HELLO WORLD!") begin
      $display("Expected `HELLO WORLD!`, got `%s`", pup);
      passed = 1'b0;
    end

    if (plw != "hello world!") begin
      $display("Expected `hello world!`, got `%s`", plw);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
