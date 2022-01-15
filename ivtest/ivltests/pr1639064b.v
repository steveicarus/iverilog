module top;
  reg [31:0] value = 1000000;
  integer  val = 100;

  initial begin
    $display("1. The value is %3d", value*100);
    $display("2. The value is %3.0f", value*100.0);
    $display("3. The value is %3.0f", 100);
    $display("4. The value is %3.0f", val);
    $display("5. The value is %3.0f", value*100000000);  // This fails!
    $finish(0);
  end
endmodule
