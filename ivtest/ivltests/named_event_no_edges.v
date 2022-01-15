module top;
  event my_event;

  // The following two line should be an error
  // You can not take the edge of a named event.
  always @(posedge my_event) $display("Posedge event.");
  always @(negedge my_event) $display("Negedge event.");
  // This should work correctly.
  always @(my_event) $display("Any event edge.");

  initial begin
    #1 ->my_event;
    #1 $display("FAILED");
 end
endmodule
