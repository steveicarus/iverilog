module event_array_test();

event my_event[3:0];

integer event_count[3:0];

generate
  genvar i;

  for (i = 0; i < 4; i = i + 1) begin
    always @(my_event[i]) begin
      $display("Got event %d", i);
      event_count[i] = event_count[i] + 1;
    end
  end
endgenerate

initial begin
  event_count[0] = 0;
  event_count[1] = 0;
  event_count[2] = 0;
  event_count[3] = 0;
  #1 ->my_event[0];
  #1 ->my_event[1];
  #1 ->my_event[2];
  #1 ->my_event[3];
  #1 ->my_event[1];
  #1 ->my_event[2];
  #1 ->my_event[3];
  #1 ->my_event[2];
  #1 ->my_event[3];
  #1 ->my_event[3];
  #1;
  if ((event_count[0] === 1)
  &&  (event_count[1] === 2)
  &&  (event_count[2] === 3)
  &&  (event_count[3] === 4))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
