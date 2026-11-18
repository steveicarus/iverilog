`timescale 1ps/1ps
module case_cond_cell(A, E, F, Z1, Z0, ZAND, ZOR);
  input A, E, F;
  output Z1, Z0, ZAND, ZOR;
  buf (Z1,A), (Z0,A), (ZAND,A), (ZOR,A);
  specify
    if (E === 1'b1) (A => Z1) = (31,37);
    if (E === 1'b0) (A => Z0) = (41,43);
    if ((E === 1'b1) && (F == 1'b0)) (A => ZAND) = (47,53);
    if ((E === 1'b1) || (F == 1'b0)) (A => ZOR) = (59,61);
  endspecify
endmodule

module top;
  reg A, E, F;
  wire z1, z0, zand, zor;
  reg [31:0] count[0:3];
  integer i;
  case_cond_cell u(A,E,F,z1,z0,zand,zor);

  task check;
    input integer index;
    input value;
    input integer rise_delay, fall_delay;
    integer event_time, delay_time;
    begin
      if ($time > 70) begin
        count[index] = count[index] + 1;
        // Input edges occur at 100, 200, ..., 800 ps. Each condition is
        // stable across its two edges. An inactive path adds no delay.
        delay_time = count[index] % 2 ? rise_delay : fall_delay;
        event_time = 100 * count[index];
        case (index)
          0, 2: if (count[index] <= 2) event_time = event_time + delay_time;
          1: if (count[index] == 3 || count[index] == 4)
               event_time = event_time + delay_time;
          3: if (count[index] <= 6) event_time = event_time + delay_time;
        endcase
        if (count[index] > 8 || $time != event_time ||
            value !== (count[index] % 2 == 1)) begin
          $display("FAILED output %0d event %0d at %0t = %b (expected %0d)",
                   index, count[index], $time, value, event_time);
          $finish(1);
        end
      end
    end
  endtask

  // Independent timing oracle: the SDF slots below are explicit constants;
  // no simulator readback or generated metadata supplies the expectations.
  always @(z1)   check(0,z1,3,5);
  always @(z0)   check(1,z0,7,11);
  always @(zand) check(2,zand,13,17);
  always @(zor)  check(3,zor,19,23);

  initial begin
    A=0; E=0; F=1;
    for (i=0; i<4; i=i+1) count[i]=0;
    $sdf_annotate("ivltests/sdf_cond_case_equality.sdf",top);
    #10; E=1; F=0;
    #90 A=1;
    #100 A=0;
    #50 E=0;
    #50 A=1;
    #100 A=0;
    #50 E=1'bx;
    #50 A=1;
    #100 A=0;
    #50; E=1'bz; F=1;
    #50 A=1;
    #100 A=0;
    #100;
    for (i=0; i<4; i=i+1)
      if (count[i] != 8) begin
        $display("FAILED output %0d count %0d",i,count[i]);
        $finish(1);
      end
    $display("PASSED");
  end
endmodule
