module top;
  integer a;

  initial begin
    a = 15'1;
    a = 15'0;
    a = 15'x;
    a = 15'X;
    a = 15'z;
    a = 15'Z;
  end
endmodule
