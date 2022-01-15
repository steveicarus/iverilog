`timescale 1ns/10ps

module top;
  reg a, pass;
  wire z;
  time edge_time;

  always @(z) begin
    if ((z === 0) && (($time - edge_time) != 4)) begin
      $display("Falling took %d, expected 4", $time - edge_time);
      pass = 1'b0;
    end
    if ((z === 1) && (($time - edge_time) != 3)) begin
      $display("Rising took %d, expected 3", $time - edge_time);
      pass = 1'b0;
    end
  end

  initial begin
    pass = 1'b1;
    $sdf_annotate("ivltests/sdf_del.sdf", top);
    #10;
    edge_time = $time;
    a = 1'b0;
    #10;
    edge_time = $time;
    a = 1'b1;
    #10 if (pass) $display("PASSED");
  end

  my_buf dut(z, a);
endmodule

module my_buf (output z, input a);
  buf (z, a);
  specify
    (a => z) = (0, 0);
  endspecify
endmodule
