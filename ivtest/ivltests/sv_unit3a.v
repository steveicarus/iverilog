int num1 = 101; string str1 = "unit1";
int num2 = 102; string str2 = "unit1";
int num3 = 103; string str3 = "unit1";

class c1;

  int num2 = 100; string str2 = "c1";

  task display;
    begin
      $display("%d from %s", num1, str1);
      $display("%d from %s", num2, str2);
    end
  endtask

endclass


module m1();

  int num2 = 112; string str2 = "m1";

  c1 obj;

  initial begin
    #1;
    obj = new;
    obj.display;
    $display("%d from %s", num1, str1);
    $display("%d from %s", num2, str2);
    $display("%d from %s", num3, str3);
    $display("%d from %s", m2.num4, m2.str4);
  end

/* This should not change the result, but Icarus ignores the order in
   which variables are declared and used.
  int num3 = 113; string str3 = "m1";
*/

endmodule


module m2();

  int num1 = 121; string str1 = "m2";
  int num2 = 122; string str2 = "m2";
  int num3 = 123; string str3 = "m2";
  int num4 = 124; string str4 = "m2";

  m1 m1inst();

endmodule
