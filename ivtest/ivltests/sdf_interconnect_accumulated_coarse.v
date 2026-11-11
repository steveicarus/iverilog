`timescale 1ps/1ps
module accumulated_source(input A,output Y); assign Y=A; endmodule
`timescale 1ns/10ps
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
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_coarse.sdf",test);
    check(0.022,0.032,20,30);
    // 2e18 local ticks fit, but multiplying by ten global ticks overflows.
    $accum_write_invalid(3);
    check(0.022,0.032,20,30);
    exercise(20,30);
    // Integer writes retain exact global ticks even below receiver precision.
    $accum_write_integer(64'd1,64'd3);
    check(0.001,0.003,1,3);
    exercise(1,3);
    // Existing real read/fill/write rounds integer-origin omitted slots again.
    // This characterizes that boundary; it does not promise exact preservation.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_partial.sdf",test);
    check(0.001,0.007,0,10);
    exercise(0,10);
    if (phases!=3) begin
      $display("FAIL phase count %0d",phases);errors=errors+1;
    end
    if (errors==0) $display("PASSED");
    $finish(0);
  end
endmodule
