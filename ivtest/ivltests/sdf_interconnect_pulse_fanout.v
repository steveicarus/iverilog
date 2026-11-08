// Cancellation belongs to a receiving connection, not the shared producer.
// Identity consumers also preserve separated X/Z values without cell delays.
`timescale 1ps/1ps

module pulse_source(input drive, output Q);
  assign Q = drive;
endmodule

module pulse_consumer(input I, output Q);
  assign Q = I;
endmodule

module test;
  reg drive;
  wire source_q, fast_q, slow_q, zero_q, peer_q;
  integer fast_changes = 0;
  integer slow_changes = 0;
  integer zero_changes = 0;
  integer peer_changes = 0;
  reg failed = 0;
  pulse_source source(drive, source_q);
  pulse_consumer fast(source_q, fast_q);
  pulse_consumer slow(source_q, slow_q);
  pulse_consumer zero(source_q, zero_q);
  pulse_consumer peer(source_q, peer_q); // deliberately unannotated

  task check;
    input integer channel;
    input integer index;
    input value;
    time expected_time;
    reg expected_value;
    reg unexpected;
    begin
      expected_time = 0;
      expected_value = 1'bx;
      unexpected = 0;
      case (channel)
        0: case (index) // 4 ps connection: the 6 ps pulse survives
           0: begin expected_time = 4; expected_value = 0; end
           1: begin expected_time = 104; expected_value = 1; end
           2: begin expected_time = 110; expected_value = 0; end
           3: begin expected_time = 144; expected_value = 1; end
           4: begin expected_time = 184; expected_value = 0; end
           5: begin expected_time = 204; expected_value = 1'bx; end
           6: begin expected_time = 244; expected_value = 1'bz; end
           7: begin expected_time = 284; expected_value = 1; end
           8: begin expected_time = 324; expected_value = 0; end
           9: begin expected_time = 364; expected_value = 1'bx; end
          10: begin expected_time = 404; expected_value = 0; end
          default: unexpected = 1;
        endcase
        1: case (index) // 10 ps connection: that same pulse is rejected
          0: begin expected_time = 10; expected_value = 0; end
          1: begin expected_time = 150; expected_value = 1; end
          2: begin expected_time = 190; expected_value = 0; end
          3: begin expected_time = 210; expected_value = 1'bx; end
          4: begin expected_time = 250; expected_value = 1'bz; end
          5: begin expected_time = 290; expected_value = 1; end
          6: begin expected_time = 330; expected_value = 0; end
          7: begin expected_time = 370; expected_value = 1'bx; end
          8: begin expected_time = 410; expected_value = 0; end
          default: unexpected = 1;
        endcase
        2, 3: case (index) // explicit zero delay and unannotated peer
           0: begin expected_time = 0; expected_value = 0; end
           1: begin expected_time = 100; expected_value = 1; end
           2: begin expected_time = 106; expected_value = 0; end
           3: begin expected_time = 140; expected_value = 1; end
           4: begin expected_time = 180; expected_value = 0; end
           5: begin expected_time = 200; expected_value = 1'bx; end
           6: begin expected_time = 240; expected_value = 1'bz; end
           7: begin expected_time = 280; expected_value = 1; end
           8: begin expected_time = 320; expected_value = 0; end
           9: begin expected_time = 360; expected_value = 1'bx; end
          10: begin expected_time = 400; expected_value = 0; end
          default: unexpected = 1;
        endcase
        default: unexpected = 1;
      endcase
      if (unexpected || $time != expected_time || value !== expected_value) begin
        $display("channel %0d change %0d: got %0t/%b, expected %0t/%b (extra=%0d)",
                 channel, index, $time, value, expected_time, expected_value, unexpected);
        failed = 1;
      end
    end
  endtask

  always @(fast_q) begin
    check(0, fast_changes, fast_q);
    fast_changes = fast_changes + 1;
  end
  always @(slow_q) begin
    check(1, slow_changes, slow_q);
    slow_changes = slow_changes + 1;
  end
  always @(zero_q) begin
    check(2, zero_changes, zero_q);
    zero_changes = zero_changes + 1;
  end
  always @(peer_q) begin
    check(3, peer_changes, peer_q);
    peer_changes = peer_changes + 1;
  end

  initial begin
    $sdf_annotate("ivltests/sdf_interconnect_pulse_fanout.sdf", test);
    drive = 0;
    #50;
    if ({fast_q, slow_q, zero_q, peer_q} !== 4'b0000 ||
        fast_changes != 1 || slow_changes != 1 || zero_changes != 1 || peer_changes != 1)
      $fatal(1, "fanout connections did not settle");
    #50 drive = 1;
    #6 drive = 0;
    #34 drive = 1;
    #40 drive = 0;
    #20 drive = 1'bx;
    #40 drive = 1'bz;
    #40 drive = 1;
    #40 drive = 0;
    #40 drive = 1'bx;
    #40 drive = 0;
    #100;
    if (failed || fast_changes != 11 || slow_changes != 9 ||
        zero_changes != 11 || peer_changes != 11 ||
        {fast_q, slow_q, zero_q, peer_q, source_q} !== 5'b00000)
      $fatal(1, "fanout history mismatch: fast=%0d slow=%0d zero=%0d peer=%0d",
             fast_changes, slow_changes, zero_changes, peer_changes);
    $display("PASSED");
    $finish(0);
  end
endmodule
