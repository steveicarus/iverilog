// Assigning to a class scope member is not supported yet. Check that it is
// reported.

class C #(int N = 4);
  static int count = 0;
endclass

module test;

  initial begin
    C#(8)::count = 1;
    $display("FAILED");
  end

endmodule
