/* I expected that p1=p2.  But the generated output looks like:

Icarus Verilog version 0.7
Copyright 1998-2003 Stephen Williams
$Name:  $

                   0 p1=x p2=StX
                   5 p1=1 p2=StX
                  10 p1=0 p2=St0
                  15 p1=1 p2=St0
                  20 p1=0 p2=St0
                  25 p1=1 p2=St0
                  30 p1=0 p2=St0
                  35 p1=1 p2=St0
                  40 p1=0 p2=St0
                  45 p1=1 p2=St0
                  50 p1=0 p2=St0
                  55 p1=1 p2=St0
                  60 p1=0 p2=St0
                  65 p1=1 p2=St0
                  70 p1=0 p2=St0
                  75 p1=1 p2=St0
                  80 p1=0 p2=St0
                  85 p1=1 p2=St0
                  90 p1=0 p2=St0
                  95 p1=1 p2=St0



Model Technology ModelSim SE vsim 5.7c Simulator 2003.03 Mar 13 2003

#                    0 p1=x p2=StX
#                    5 p1=1 p2=StX
#                   10 p1=0 p2=St0
#                   15 p1=1 p2=St0
#                   20 p1=0 p2=St0
#                   25 p1=1 p2=St0
#                   30 p1=0 p2=St0
#                   35 p1=1 p2=St0
#                   40 p1=0 p2=St0
#                   45 p1=1 p2=St0
#                   50 p1=0 p2=St0
#                   55 p1=1 p2=St0
#                   60 p1=0 p2=St0
#                   65 p1=1 p2=St0
#                   70 p1=0 p2=St0
#                   75 p1=1 p2=St0
#                   80 p1=0 p2=St0
#                   85 p1=1 p2=St0
#                   90 p1=0 p2=St0
#                   95 p1=1 p2=St0
#
*/
`timescale 1 ns / 1 ns

module pulse;
reg p1,p2;


  initial
  begin
    $monitor("%t p1=%b p2=%v",$time,p1,p2);
    #101 $finish(0);
  end


  initial
    repeat(10)
    begin
      #5 p1=1'b1;
      #5 p1=1'b0;
    end


  initial
    repeat(10) single_pulse(p2);


  task single_pulse;
  output p;
  begin
    #5 p=1'b1;
    #5 p=1'b0;
  end
  endtask

endmodule
