
program main;

   function int sum_array(bit[7:0] array[]);
      int idx;
      sum_array = 0;
      for (idx = 0 ; idx < array.size() ; idx = idx+1)
	sum_array += array[idx];
   endfunction // sum_array

   bit [7:0] obj[];
   int	     foo;
   initial begin
      foo = sum_array('{});
      if (foo !== 0) begin
	 $display("FAILED -- sum of empty array returns %0d", foo);
	 $finish;
      end

      obj = new[3];
      obj[0] = 1;
      obj[1] = 2;
      obj[2] = 3;
      foo = sum_array(obj);
      if (foo !== 6) begin
	 $display("FAILED -- sum of '{1,2,3} is %0d", foo);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endprogram // main
