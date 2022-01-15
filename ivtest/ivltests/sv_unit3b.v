int num1 = 201; string str1 = "unit2";
int num2 = 202; string str2 = "unit2";
int num3 = 203; string str3 = "unit2";

module m3();

  int num2 = 232; string str2 = "m3";

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
  int num3 = 113; string str3 = "m3";
*/

endmodule


module m4();

  int num1 = 241; string str1 = "m4";
  int num2 = 242; string str2 = "m4";
  int num3 = 243; string str3 = "m4";
  int num4 = 244; string str4 = "m4";

  m3 m3inst();

endmodule
