module returnval();

function real my_func(input real value);

time	last_time;

begin
  if ($time !== last_time) begin
    last_time = $time;
    my_func = value;
  end
  else
    my_func = my_func + 1;
end

endfunction

wire real my_func_value = my_func.my_func;

initial begin
// Icarus can't do this:
//$monitor("saw my_func value: %f", my_func.my_func);
// Do this instead: 
  $monitor("saw my_func value: %f", my_func_value);

  $display("got my_func value: %f", my_func(10.0));
  $display("got my_func value: %f", my_func(10.0));
  #1;
  $display("got my_func value: %f", my_func(20.0));
  $display("got my_func value: %f", my_func(20.0));
  #1;
  $display("got my_func value: %f", my_func(30.0));
  $display("got my_func value: %f", my_func(30.0));
end

endmodule
