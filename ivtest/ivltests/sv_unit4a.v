localparam num1 = 101, str1 = "unit1";
localparam num2 = 102, str2 = "unit1";
localparam num3 = 103, str3 = "unit1";

class c1;

  const int num2 = 100; const string str2 = "c1";

  task display;
    begin
      $display("%d from %s", num1, str1);
      $display("%d from %s", num2, str2);
    end
  endtask

endclass


module m1();

  localparam num2 = 112, str2 = "m1";

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
  localparam num3 = 113, str3 = "m1";
*/

endmodule


module m2();

  localparam num1 = 121, str1 = "m2";
  localparam num2 = 122, str2 = "m2";
  localparam num3 = 123, str3 = "m2";
  localparam num4 = 124, str4 = "m2";

  m1 m1inst();

endmodule
