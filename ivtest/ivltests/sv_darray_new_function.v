// Check that a dynamic array constructor accepts a function result.

module test;

  typedef int array_t[];
  array_t a;

  function array_t make;
    make = new[1];
    make[0] = 42;
  endfunction

  initial begin
    a = new[1](make());

    if (a[0] !== 42) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
