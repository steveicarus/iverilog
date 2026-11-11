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
    // An initial increment starts from zero; no ABSOLUTE has occurred.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_initial.sdf",test);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,50.0,40,50);
    exercise(40,50);
    // The maintainer sequence within one file.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_same.sdf",test);
    check(40.0,50.0,40,50);
    exercise(40,50);
    // Separate files and actual zero-delay propagation between updates.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    exercise(0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,50.0,40,50);
    exercise(40,50);
    // All-null ABSOLUTE and INCREMENT retain negative fields.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_null.sdf",test);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,50.0,40,50);
    exercise(40,50);
    // Partial ABSOLUTE preserves the omitted negative rise value.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_partial.sdf",test);
    check(-10.0,7.0,0,7);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,77.0,40,77);
    exercise(40,77);
    // Partial INCREMENT supplies no update for the omitted rise value.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_partial_increment.sdf",test);
    check(-10.0,-15.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,55.0,40,55);
    exercise(40,55);
    // An explicit integer zero replacement discards the previous signed state.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    $accum_write_integer(64'd0,64'd0);
    check(0.0,0.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(50.0,70.0,50,70);
    exercise(50,70);
    // Integer replacements also refresh real values used by later SDF reads.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    $accum_write_integer(64'd3,64'd4);
    check(3.0,4.0,3,4);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(53.0,74.0,53,74);
    exercise(53,74);
    // A direct scaled-real write uses the same raw/effective representation.
    $accum_write_real(-10.0,-20.0);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(40.0,50.0,40,50);
    exercise(40,50);
    // A later ABSOLUTE replaces the supplied values.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_absolute.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_negative.sdf",test);
    check(-10.0,-20.0,0,0);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_reset.sdf",test);
    check(3.0,4.0,3,4);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_plus.sdf",test);
    check(53.0,74.0,53,74);
    exercise(53,74);
    // Preserve fractions across writes, not just intermediate negative values.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_fraction.sdf",test);
    check(0.8,0.8,1,1);
    exercise(1,1);
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_fraction_signed.sdf",test);
    check(2.2,3.2,2,3);
    exercise(2,3);
    // Raw signed/fractional ABSOLUTE 1-value expansion.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_short1.sdf",test);
    if (!$accum_check_slots(0)) errors=errors+1;
    exercise(0,0);
    // Raw signed/fractional ABSOLUTE 2-value expansion.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_short2.sdf",test);
    if (!$accum_check_slots(1)) errors=errors+1;
    exercise(0,3);
    // Raw signed/fractional ABSOLUTE 3-value expansion.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_short3.sdf",test);
    if (!$accum_check_slots(2)) errors=errors+1;
    exercise(0,3);
    // Raw signed/fractional ABSOLUTE 6-value expansion.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_short6.sdf",test);
    if (!$accum_check_slots(3)) errors=errors+1;
    exercise(0,3);
    // Raw signed/fractional ABSOLUTE 12-value expansion.
    $sdf_annotate("ivltests/sdf_interconnect_accumulated_short12.sdf",test);
    if (!$accum_check_slots(4)) errors=errors+1;
    exercise(0,3);
    if (phases!=18) begin
      $display("FAIL phase count %0d",phases);errors=errors+1;
    end
    if (errors==0) $display("PASSED");
    $finish;
  end
endmodule
