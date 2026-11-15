// A class with a parameter that has no default value needs a value for each
// of those parameters. Check that using it without them is reported.

class C #(type T, int N);
  T data;
endclass

module test;

  C a;
  C #(int) b;
  C #() c;
  C #(.N(2)) d;

  initial begin
    $display("FAILED");
  end

endmodule
