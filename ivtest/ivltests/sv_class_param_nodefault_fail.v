// Class parameters without a default value are not supported yet. Check that
// they are reported.

class C #(int N);
endclass

module test;

  C #(8) a;

  initial begin
    $display("FAILED");
  end

endmodule
