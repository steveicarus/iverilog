
program main;

   function string sum_array(string array[]);
      int idx;
      sum_array = "";
      for (idx = 0 ; idx < array.size() ; idx = idx+1)
	sum_array = {sum_array, array[idx]};
   endfunction // sum_array

   string obj[];
   string foo;
   initial begin
      foo = sum_array('{});
      if (foo != "") begin
	 $display("FAILED -- sum of empty array returns %0s", foo);
	 $finish;
      end

      obj = new[3];
      obj = '{"1", "2", "3"};
      foo = sum_array(obj);
      if (foo != "123") begin
	 $display("FAILED -- sum of '{\"1\",\"2\",\"3\"} is %0s", foo);
	 $finish;
      end

      obj = new[3] ('{"A", "B", "C"});
      foo = sum_array(obj);
      if (foo != "ABC") begin
	 $display("FAILED -- sum of '{\"A\",\"B\",\"C\"} is %0s", foo);
	 $finish;
      end

      obj = new[3] ("A");
      foo = sum_array(obj);
      if (foo != "AAA") begin
	 $display("FAILED -- sum of \"AAA\" is %0s", foo);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endprogram // main
