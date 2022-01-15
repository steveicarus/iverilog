`timescale 1ns/1ns

module top;
  reg [7:0] data;
  reg [9:0] odata;
  reg sout, clk;
  integer lp;

  initial begin
    data = 8'h55;

    #0 $display("Printing the byte %b with a header.", data);
    $write("Bad - ");
    odata = 10'b1x00000000;
    odata[7:0] = data;
    send_byte(odata);
//    #0 send_byte(odata);  // This fixes things, but should not be needed!
    $write(", ok - ");
    send_byte(odata);
    $display(".");

    #1 data = 0;
    #1 $finish(0);
  end

  // Print a byte of data.
  task send_byte;
    input [9:0] sndbyte;

    begin
      $write("%b", sndbyte);
    end
  endtask

endmodule
