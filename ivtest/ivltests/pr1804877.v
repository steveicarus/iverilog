module test;
  reg [128:1] str [0:0];
  reg [31:0] idx;

  initial begin
    str[0] = "test_counter";
    idx = 0;
    $write("String is %s\n", str[0]); // This works.
    $write("String is %s\n", str[idx]); // This fails.
  end
endmodule
