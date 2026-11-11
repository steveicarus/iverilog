`timescale 1ps/1ps
module accumulated_source(input A,output Y); assign Y=A; endmodule
module accumulated_load(input A,output Y); assign Y=A; endmodule
`timescale 1ps/1ps
module test;
  reg a;
  wire n,q;
  integer errors,events,phases;
  reg active;
  reg [63:0] expected_rise,expected_fall;
  accumulated_source src(a,n);
  accumulated_load sink(n,q);

  task check;
    input real rise_raw,fall_raw;
    input [63:0] rise_ticks,fall_ticks;
    begin
      if (!$accum_check(rise_raw,fall_raw,rise_ticks,fall_ticks)) errors=errors+1;
    end
  endtask

  task exercise;
    input [63:0] rise_ticks,fall_ticks;
    begin
      events=0;active=1;
      #100;expected_rise=$time+rise_ticks;a=1;
      #100;expected_fall=$time+fall_ticks;a=0;
      #100;active=0;phases=phases+1;
      if (events!=2) begin
        $display("FAIL phase %0d event count %0d",phases,events);errors=errors+1;
      end
    end
  endtask

  always @(q) if (active) begin
    if (events==0 && (q!==1'b1 || $time!==expected_rise) ||
        events==1 && (q!==1'b0 || $time!==expected_fall) || events>=2) begin
      $display("FAIL phase %0d event %0d at %0t value %b",phases,events,$time,q);
      errors=errors+1;
    end
    events=events+1;
  end

  initial begin
    errors=0;events=0;phases=0;active=0;a=0;
    #1;
    // Discover the initially zero-delay object, then establish receiver history.
    check(0.0,0.0,0,0);
    #9 a=1;
    #90 a=0;
    #100;
    $accum_write_real(-10.0,-20.0);
    check(-10.0,-20.0,0,0);
    $accum_write_invalid(0);
    check(-10.0,-20.0,0,0);
    $accum_write_invalid(1);
    check(-10.0,-20.0,0,0);
    $accum_write_invalid(2);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,50.0,40,50);
    exercise(40,50);
    // Neither integer write nor integer read may pass through double.
    $accum_write_integer(64'h0020000000000001,64'hffffffffffffffff);
    check(9007199254740992.0,18446744073709551616.0,64'h0020000000000001,64'hffffffffffffffff);
    // Do not schedule events with the large delays; replace them first.
    $accum_write_integer(64'd3,64'd4);
    check(3.0,4.0,3,4);
    exercise(3,4);
    if (phases!=2) begin
      $display("FAIL phase count %0d",phases);errors=errors+1;
    end
    if (errors==0) $display("PASSED");
    $finish(0);
  end
endmodule
