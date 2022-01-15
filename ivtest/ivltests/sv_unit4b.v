parameter num1 = 201, str1 = "unit2";
parameter num2 = 202, str2 = "unit2";
parameter num3 = 203, str3 = "unit2";

module m3();

  parameter num2 = 232, str2 = "m3";

  initial begin
    #2; // allow m1 to go first
    m2.m1inst.obj.display;
    $display("%d from %s", num1, str1);
    $display("%d from %s", num2, str2);
    $display("%d from %s", num3, str3);
    $display("%d from %s", m4.num4, m4.str4);
  end

/* This should not change the result, but Icarus ignores the order in
   which variables are declared and used.
  parameter num = 113, str3 = "m3";
*/

endmodule


module m4();

  parameter num1 = 241, str1 = "m4";
  parameter num2 = 242, str2 = "m4";
  parameter num3 = 243, str3 = "m4";
  parameter num4 = 244, str4 = "m4";

  m3 m3inst();

endmodule
