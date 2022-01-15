module example;
  function simple_func;
    input in;
    begin
      simple_func = in;
    end
  endfunction

  reg x = 0;
  initial begin
    x = simple_func(x,x);
    $finish;
  end
endmodule
