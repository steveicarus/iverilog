// These don't do anything useful, but check for compiler errors.

module test();

integer i;

always begin
  for (i = 0; i < 10; i = i + 1) ;
end

always begin
  repeat (1) ;
end

always begin
  while (1) ;
end

always begin
  do ; while (1);
end

always begin
  forever ;
end

endmodule
