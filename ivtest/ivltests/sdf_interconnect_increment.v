// SDF increments apply to current receiver delays, not replacement values.
// Literal expectations below are independent of the annotation readback.
`timescale 1ps/1ps
module increment_source(input A, output Y);
  buf (Y,A);
endmodule
module increment_load(input A, output Y);
  buf (Y,A);
endmodule
`timescale 1ns/1ps
module increment_other(input A, output Y);
  buf (Y,A);
endmodule
`timescale 1ps/1ps
module test;
  reg a;
  wire n,q,other;
  integer errors,main_events,other_events;
  reg [63:0] expected[0:9];
  reg [63:0] other_expected[0:9];
  increment_source src(a,n);
  increment_load sink(n,q);
  increment_other sibling(n,other);

  task check;
    input integer rise_delay,fall_delay;
    begin
      if (!$check_increment_paths(rise_delay,fall_delay,11,13)) errors=errors+1;
    end
  endtask

  initial begin
    errors=0; main_events=0; other_events=0; a=0;
    expected[0]=27;expected[1]=50;expected[2]=85;expected[3]=110;
    expected[4]=150;expected[5]=166;expected[6]=190;expected[7]=210;
    expected[8]=243;expected[9]=264;
    other_expected[0]=31;other_expected[1]=53;other_expected[2]=91;other_expected[3]=113;
    other_expected[4]=151;other_expected[5]=173;other_expected[6]=201;other_expected[7]=223;
    other_expected[8]=251;other_expected[9]=273;
    #1;
    $sdf_annotate("ivltests/sdf_interconnect_increment_absolute.sdf",test);
    check(5,7);
    $sdf_annotate("ivltests/sdf_interconnect_increment_add.sdf",test);
    check(7,10);
    #19 a=1;
    #20 a=0;
    #20;
    $sdf_annotate("ivltests/sdf_interconnect_increment_partial.sdf",test);
    check(5,10);
    #20 a=1;
    #20 a=0;
    #30;
    $sdf_annotate("ivltests/sdf_interconnect_increment_mixed.sdf",test);
    check(10,6);
    #10 a=1;
    #20 a=0;
    #20;
    $sdf_annotate("ivltests/sdf_interconnect_increment_clamp.sdf",test);
    check(0,0);
    #10 a=1;
    #20 a=0;
    #20;
    $sdf_annotate("ivltests/sdf_interconnect_increment_reset.sdf",test);
    check(3,4);
    #10 a=1;
    #20 a=0;
    #20;
    if(main_events!=10 || other_events!=10)begin
      $display("FAIL event counts %0d %0d",main_events,other_events);errors=errors+1;
    end
    if(errors==0)$display("PASSED");
  end
  always @(q) if($time>1)begin
    if(main_events>=10 || $time!==expected[main_events] || q!==(main_events%2==0))begin
      $display("FAIL receiver event %0d %0t %b",main_events,$time,q);errors=errors+1;
    end
    main_events=main_events+1;
  end
  always @(other) if($time>1)begin
    if(other_events>=10 || $time!==other_expected[other_events] || other!==(other_events%2==0))begin
      $display("FAIL sibling event %0d %0t %b",other_events,$time,other);errors=errors+1;
    end
    other_events=other_events+1;
  end
endmodule
