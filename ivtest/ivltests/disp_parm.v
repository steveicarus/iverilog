/* From PR#516 */
module top ();

  parameter GEORGE = 8'd5;
  parameter HARRY = 10;

  initial begin
    #1;
    $display("decimal GEORGE: %0d, HARRY: %0d",GEORGE, HARRY);
    $display("binary GEORGE: 'b%0b, HARRY: 'b%0b",GEORGE, HARRY);
    $finish(0);
  end

endmodule
