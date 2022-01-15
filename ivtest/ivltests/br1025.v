function int f (int arg);

  begin:b // comment to remove bug

    int i;
    f = 0;
    for (i=0; i<arg; i++)
      f += i;

  end	 // comment to remove bug

endfunction


module bug;

int sum;

assign sum = f(10);

initial begin
  #0 $display("sum = %0d", sum);
  if (sum === 45)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

