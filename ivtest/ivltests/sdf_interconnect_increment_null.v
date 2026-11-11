// A short increment list must retain each transition's distinct prior delay.
`timescale 1ps/1ps
module increment_source(input A, output Y); assign Y=A; endmodule
module increment_load(input A, output Y); assign Y=A; endmodule
module test;
  reg a;
  wire n,q;
  integer errors,events;
  increment_source src(a,n);
  increment_load sink(n,q);
  initial begin
    errors=0;events=0;a=0;
    #1;
    $sdf_annotate("ivltests/sdf_interconnect_increment_null.sdf",test);
    if (!$check_increment_pair(5,7)) errors=errors+1;
    #19 a=1;
    #20 a=0;
    #20;
    if(events!=2)begin $display("FAIL event count %0d",events);errors=errors+1;end
    if(errors==0)$display("PASSED");
  end
  always @(q) if($time>1)begin
    if(events==0 && ($time!==25 || q!==1'b1) ||
       events==1 && ($time!==47 || q!==1'b0) || events>=2)begin
      $display("FAIL delivery %0d at %0t value %b",events,$time,q);errors=errors+1;
    end
    events=events+1;
  end
endmodule
