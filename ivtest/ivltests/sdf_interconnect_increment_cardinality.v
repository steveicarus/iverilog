// SDF lists are expanded before adding their increments to existing delays.
// Deliberately non-derived initial X delays detect recomputation from updated
// binary/Z delays. Literal expectations do not depend on VPI readback.
`timescale 1ps/1ps
module increment_source(input A, output Y); assign Y=A; endmodule
module increment_load(input A, output Y); assign Y=A; endmodule
module test;
  reg a;
  wire n,q;
  integer errors,events,phase,step;
  reg [63:0] expected[0:107];
  reg states[0:11];
  reg [31:0] slots[0:11];
  reg [63:0] launched;
  increment_source src(a,n);
  increment_load sink(n,q);
  task exercise;
    input integer which;
    begin
      phase=which;events=0;
      if(!$check_increment_cardinality(which))errors=errors+1;
      for(step=0;step<12;step=step+1)begin
        #200;launched=$time;a=states[step];
      end
      #200;
      if(events!=12)begin
        $display("FAIL phase %0d event count %0d",phase,events);errors=errors+1;
      end
    end
  endtask
  initial begin
    errors=0;events=0;phase=-1;step=0;a=0;launched=0;
    expected[0]=13;
    expected[1]=28;
    expected[2]=39;
    expected[3]=43;
    expected[4]=58;
    expected[5]=72;
    expected[6]=73;
    expected[7]=85;
    expected[8]=102;
    expected[9]=106;
    expected[10]=118;
    expected[11]=129;
    expected[12]=13;
    expected[13]=28;
    expected[14]=40;
    expected[15]=43;
    expected[16]=56;
    expected[17]=72;
    expected[18]=73;
    expected[19]=85;
    expected[20]=100;
    expected[21]=106;
    expected[22]=116;
    expected[23]=129;
    expected[24]=13;
    expected[25]=28;
    expected[26]=40;
    expected[27]=48;
    expected[28]=64;
    expected[29]=80;
    expected[30]=73;
    expected[31]=90;
    expected[32]=102;
    expected[33]=114;
    expected[34]=124;
    expected[35]=134;
    expected[36]=12;
    expected[37]=25;
    expected[38]=40;
    expected[39]=45;
    expected[40]=58;
    expected[41]=73;
    expected[42]=78;
    expected[43]=91;
    expected[44]=106;
    expected[45]=111;
    expected[46]=124;
    expected[47]=139;
    expected[48]=13;
    expected[49]=23;
    expected[50]=39;
    expected[51]=43;
    expected[52]=53;
    expected[53]=67;
    expected[54]=73;
    expected[55]=85;
    expected[56]=97;
    expected[57]=101;
    expected[58]=113;
    expected[59]=127;
    expected[60]=13;
    expected[61]=23;
    expected[62]=40;
    expected[63]=48;
    expected[64]=53;
    expected[65]=80;
    expected[66]=73;
    expected[67]=90;
    expected[68]=97;
    expected[69]=101;
    expected[70]=113;
    expected[71]=134;
    expected[72]=12;
    expected[73]=23;
    expected[74]=40;
    expected[75]=41;
    expected[76]=58;
    expected[77]=67;
    expected[78]=78;
    expected[79]=83;
    expected[80]=106;
    expected[81]=101;
    expected[82]=124;
    expected[83]=127;
    expected[84]=11;
    expected[85]=23;
    expected[86]=37;
    expected[87]=41;
    expected[88]=53;
    expected[89]=67;
    expected[90]=71;
    expected[91]=83;
    expected[92]=97;
    expected[93]=101;
    expected[94]=113;
    expected[95]=127;
    expected[96]=11;
    expected[97]=23;
    expected[98]=37;
    expected[99]=41;
    expected[100]=53;
    expected[101]=67;
    expected[102]=71;
    expected[103]=83;
    expected[104]=97;
    expected[105]=101;
    expected[106]=113;
    expected[107]=127;
    states[0]=1'b1; slots[0]=0;
    states[1]=1'b0; slots[1]=1;
    states[2]=1'bz; slots[2]=2;
    states[3]=1'b0; slots[3]=5;
    states[4]=1'bx; slots[4]=6;
    states[5]=1'b1; slots[5]=7;
    states[6]=1'bz; slots[6]=4;
    states[7]=1'b1; slots[7]=3;
    states[8]=1'bx; slots[8]=8;
    states[9]=1'bz; slots[9]=10;
    states[10]=1'bx; slots[10]=11;
    states[11]=1'b0; slots[11]=9;
    #1;
    // Establish receiver history after creation before testing all transitions.
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    a=1;
    #200 a=0;
    #200;
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_two.sdf",test);
    exercise(0);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_three.sdf",test);
    exercise(1);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_six.sdf",test);
    exercise(2);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_twelve.sdf",test);
    exercise(3);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_partial_two.sdf",test);
    exercise(4);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_partial_six.sdf",test);
    exercise(5);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_partial_twelve.sdf",test);
    exercise(6);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_null_six.sdf",test);
    exercise(7);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_reset.sdf",test);
    $sdf_annotate("ivltests/sdf_interconnect_increment_cardinality_null_twelve.sdf",test);
    exercise(8);
    if(errors==0)$display("PASSED");
  end
  always @(q) if(phase>=0)begin
    if(events>=12 || q!==states[events] ||
       $time!==launched+expected[phase*12+slots[events]])begin
      $display("FAIL phase %0d event %0d at %0t value %b",phase,events,$time,q);
      errors=errors+1;
    end
    events=events+1;
  end
endmodule
