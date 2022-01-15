module pr3534333();

/* Check compiler accepts null statements in blocks */

integer count = 0;

initial begin
end

initial begin
  (* my_attr = 0 *) ;
end

initial begin
  (* my_attr = 0 *) ;
  #1 count = count + 1;
end

initial begin
  #2 count = count + 1;
  (* my_attr = 0 *) ;
end

initial begin
  ;
  #3 count = count + 1;
  ;
end

initial begin
  #4;;
  if (count === 3)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
