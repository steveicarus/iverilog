`timescale 1ps/1ps
// Translation must retain source identities, not just Boolean equivalence.
// The alias and escaped names also exercise the original bare-COND subset.
module cond_vlog95_cell(A, E, F, G, H, ZB, ZA, ZE, ZF, ZL, ZR, ZH, ZQ, ZK, ZU);
  input A, E, F, G, H;
  output ZB, ZA, ZE, ZF, ZL, ZR, ZH, ZQ, ZK, ZU;
  wire enable_alias = E;
  wire \E==1'b1 = H;
  wire \gate"quote = H;
  wire \gate\back = H;
  buf (ZB,A), (ZA,A), (ZE,A), (ZF,A), (ZL,A), (ZR,A),
      (ZH,A), (ZQ,A), (ZK,A), (ZU,A);
  specify
    if (E) (A => ZB) = (1,1);
    if (enable_alias) (A => ZA) = (1,1);
    if (E == 1'b1) (A => ZE) = (1,1);
    if (F == 1'h0) (A => ZF) = (1,1);
    if (((E == 1'b1) && (F == 1'b0)) && (G == 1'b1)) (A => ZL) = (1,1);
    if ((E == 1'b1) && ((F == 1'b0) && (G == 1'b1))) (A => ZR) = (1,1);
    if (\E==1'b1 ) (A => ZH) = (1,1);
    if (\gate"quote == 1'b1) (A => ZQ) = (1,1);
    if (\gate\back == 1'b1) (A => ZK) = (1,1);
    (A => ZU) = (1,1);
  endspecify
endmodule
module top;
  reg A, E, F, G, H;
  wire z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;
  reg [31:0] count[0:9];
  integer i;
  cond_vlog95_cell u(A,E,F,G,H,z0,z1,z2,z3,z4,
                    z5,z6,z7,z8,z9);
  task check;
    input integer index;
    input value;
    input integer rise_time, fall_time;
    begin
      if ($time > 50) begin
        count[index] = count[index] + 1;
        if (!((count[index] == 1 && value === 1'b1 && $time == rise_time) ||
              (count[index] == 2 && value === 1'b0 && $time == fall_time))) begin
          $display("FAILED output %0d event %0d at %0t = %b",
                   index, count[index], $time, value);
          $finish(1);
        end
      end
    end
  endtask
  // Independent oracle: input edges are at 100/200 ps; each pair below is
  // that edge plus the corresponding SDF rise/fall delay, not parsed metadata.
  always @(z0) check(0,z0,103,205);
  always @(z1) check(1,z1,107,209);
  always @(z2) check(2,z2,111,213);
  always @(z3) check(3,z3,115,217);
  always @(z4) check(4,z4,119,221);
  always @(z5) check(5,z5,123,225);
  always @(z6) check(6,z6,127,229);
  always @(z7) check(7,z7,131,233);
  always @(z8) check(8,z8,135,237);
  always @(z9) check(9,z9,139,241);
  initial begin
    A=0; E=0; F=1; G=0; H=0;
    for (i=0; i<10; i=i+1) count[i]=0;
    $sdf_annotate("ivltests/sdf_cond_vlog95.sdf",top);
    #10; E=1; F=0; G=1; H=1;
    #90 A=1;
    #100 A=0;
    #100;
    for (i=0; i<10; i=i+1)
      if (count[i] != 2) begin
        $display("FAILED output %0d count %0d",i,count[i]);
        $finish(1);
      end
    $display("PASSED");
  end
endmodule
