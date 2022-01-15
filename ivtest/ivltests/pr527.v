//  Icarus 0.6 AND snapshot 20020728
//  -----------------------------
//  (1) force to nets not supported
//  (2) comment the force statement and the release statement will cause
//      the compiler to fail silently (no messages, no a.out)
//
//  Icarus snapshot 20020817
//  ------------------------
//  Runs fine IFF the whole of a bus is set, cannot force individual bits
//  (Fails with a rather incomprehensible error)
//
//  To run this, incant:
//                     iverilog tt.v
//                     (adding -Wall doesn't help)
//                     vvp a.out (if a.out is generated!)
//
//
//  Veriwell
// ---------
//  Runs fine IFF the whole of a bus is set, cannot force individual bits
//  & crashes if a release of an individual bit is attempted.
//
//  To run this, incant:
//                     veridos tt.v  (or use the GUI)
//

module top ();

  reg [31:0] ii;
  reg fail;
  reg [1:0] a;
  wire [1:0] junk = a;
  wire [1:0] junkbus = a;

  initial begin
    a = 2'b01;
    #5; a = 2'b10;
    #10; a = 2'b11;
  end

  initial  begin
    #2;
    force junk = 0;
    force junkbus[0] = 0;
    #10;
    release junk;
    #5;
    release junkbus[0];
  end

  initial begin
    $display("");
    $display("expecting junk,junkbus to be 1 at T=1");
    $display("then changing to 0 at T=2");
    $display("then junk is 0 from T=3 to T=11, while");
    $display("junkbus changes to 2 at T=5 and remains 2 through to T=16");
    $display("junk changes to 2 at T=12");
    $display("then 2 from T=13 to T=14");
    $display("then changing to 3 at T=15");
    $display("then 3 from T=16 on");
    $display("junkbus changes to 3 at T=17 and remains 3 from then on");
    $display("");
    for(ii = 0; ii < 20; ii = ii + 1) begin
      #0; // avoid race
      // junk
      if((ii == 1) && (junk !== 1)) fail = 1;
      if((ii > 2) && (ii < 12) && (junk !== 0)) fail = 1;
      if((ii > 12) && (ii < 14) && (junk !== 2'b10)) fail = 1;
      if((ii > 15) && (junk !== 2'b11)) fail = 1;
      // junkbus
      if((ii == 1) && (junkbus !== 2'b01)) fail = 1;
      if((ii > 2) && (ii < 4) && (junkbus !== 2'b00)) fail = 1;
      if((ii > 5) && (ii < 17) && (junkbus !== 2'b10)) fail = 1;
      if((ii > 17) && (junkbus !== 2'b11)) fail = 1;
      $display("time: %0t, a: %b, junk: %b, junkbus: %b",$time,a,junk,junkbus);
      #1;
    end
    if(fail) $display("\n\t--------- force test failed ---------\n");
    else     $display("\n\t--------- force test passed ---------\n");
  end

endmodule
