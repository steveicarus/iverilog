// A negative increment is valid when the accumulated delay remains positive.
// Readback and literal output times are checked independently.
`timescale 1ps/1ps
module subtract_source(input A,output Y); assign Y=A; endmodule
module subtract_load(input A,output Y); assign Y=A; endmodule
module test;
  reg a,measuring;
  wire n,q;
  integer errors,events;
  subtract_source src(a,n);
  subtract_load sink(n,q);

  initial begin
    a=0;measuring=0;errors=0;events=0;
    #1;
    $sdf_annotate("ivltests/sdf_interconnect_increment_subtract.sdf",test);
    if (!$accum_check(10.0,10.0,64'd10,64'd10)) errors=errors+1;
    // Prime the inserted path and settle its receiver before scored events.
    #9 a=1;
    #40 a=0;
    #50 measuring=1;a=1;
    #100 a=0;
    #100;
    if (events!=2) begin
      $display("FAIL event count %0d",events);errors=errors+1;
    end
    if (errors==0) $display("PASSED");
    $finish(0);
  end

  always @(q) if (measuring) begin
    if (events==0 && ($time!==110 || q!==1'b1) ||
        events==1 && ($time!==210 || q!==1'b0) || events>=2) begin
      $display("FAIL event %0d at %0t value %b",events,$time,q);
      errors=errors+1;
    end
    events=events+1;
  end
endmodule
