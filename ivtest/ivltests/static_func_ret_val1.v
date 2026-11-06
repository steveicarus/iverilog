module returnval();

function [31:0] random(input dummy);

time	last_time;

begin
  if ($time !== last_time) begin
    last_time = $time;
    random = $random;
  end
  else
    random = random + 1;
end

endfunction

wire [31:0] random_value = random.random;

initial begin
// Icarus can't do this:
// $monitor("saw random value: %x", random.random);
// Do this instead:
  $monitor("saw random value: %x", random_value);

  $display("got random value: %x", random(0));
  $display("got random value: %x", random(0));
  #1;
  $display("got random value: %x", random(0));
  $display("got random value: %x", random(0));
  #1;
  $display("got random value: %x", random(0));
  $display("got random value: %x", random(0));
end

endmodule
