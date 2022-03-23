// Check that it is possible to overwrite a type identifier declared in a higher
// level scope. Check that this works if the new type is an array type.

typedef logic [3:0] T;
T x;

module test;

  typedef logic [7:0] T[1:0];
  T y;

  initial begin
    y[0] = 8'hff;
    y[1] = 8'hfe;

    if ($bits(T) == 16 && $size(x) == 4 && $size(y) == 2 &&
        y[0] == 8'hff && y[1] == 8'hfe) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
