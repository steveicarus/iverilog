module test;

typedef enum reg { FALSE = 1'b0, TRUE = 1'b1 } boolean;

boolean flag;

initial begin
  #1 $display("%b", flag);
  if (flag === TRUE)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
