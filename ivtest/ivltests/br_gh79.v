module test();

reg [3:0] BadNumber;

initial begin
  BadNumber = 4'b_;
  BadNumber = 4'b_1;
  BadNumber = 4'b1_;
  BadNumber = 4'o_;
  BadNumber = 4'o_1;
  BadNumber = 4'o1_;
  BadNumber = 4'd_;
  BadNumber = 4'd_1;
  BadNumber = 4'd1_;
  BadNumber = 4'h_;
  BadNumber = 4'h_1;
  BadNumber = 4'h1_;
end

endmodule
