// Check that it is an error to not declare the data type in for loops, even
// when using var

module test;

  initial begin
    // The data type is not optional in a for loop, even when using var
    for (var [7:0] i = 0; i < 10; i++) begin
    end
    $display("FAILED");
  end

endmodule
