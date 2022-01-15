`timescale 1ns/1ns

module hct74245(
  input dir,
  input nOE,
  inout [7:0] A,
  inout [7:0] B,
  output [7:0] C // Added to demonstrate that a straight dependency timing works correctly
);

  // HCT typical @ 5v according to https://assets.nexperia.com/documents/data-sheet/74HC_HCT245.pdf

  specify
    (A *> C) = 100; // This delay works OK
    (A *> B) = 10;  // The rest of these delays do not work
    (B *> A) = 10; // not respected
    (dir *> A) = 16;// not respected
    (dir *> B) = 16;// not respected
    (nOE *> A) = 16;// not respected
    (nOE *> B) = 16;// not respected
  endspecify

  assign A=nOE? 8'bzzzzzzzz : dir?8'bzzzzzzzz:B;
  assign B=nOE? 8'bzzzzzzzz : dir?A:8'bzzzzzzzz;
  assign C=A; // THIS IS DELAYED BY 100

  // HAVE TO USE THIS APPROACH TO MAKE TIMINGS WORK AT ALL
  // assign #16 A=nOE? 8'bzzzzzzzz :dir?8'bzzzzzzzz:B;
  // assign #16 B=nOE? 8'bzzzzzzzz :dir?A: 8'bzzzzzzzz;

endmodule

module tb();

  tri [7:0]A;
  tri [7:0]B;

  tri [7:0]C; // 'C' IS NOT PART OF ORIGINAL DESIGN - HOWEVER THIS TIMING IS RESPECTED COS THERE ARE NO CONDITIONALS IN THE ASSIGNMENT

  reg [7:0] Vb=8'b00000000;
  reg [7:0] Va=8'b11111111;

  reg dir;
  reg nOE;

  assign B=Vb;
  assign A=Va;

  hct74245 buf245(.A, .B, .dir, .nOE);

  integer timer;

  reg failed = 0;

  initial begin
    $display("disable output , set dir a->b");
    dir <= 1; // A->B
    nOE <= 1;
    Va=8'b11111111;
    Vb=8'bzzzzzzzz;

    #50 // time to settle

    // NOW throw outputs on and time how long it takes for expected output to appear.

    // It should take 16 to propagate from nOE -> B but the change is instantaneous

    $display("enable output - B will change immediately");

    timer=$time;
    nOE <= 0;
    wait(B === 8'b11111111);
    if ($time - timer < 16) begin
      $display("%6d", $time, " ERROR TOO QUICK - EXPECTED nOE->B = 16ns - BUT TOOK %-d", ($time - timer));
      failed = 1;
    end
    else
      $display("%6d", $time, " OK - TOOK %-d", ($time - timer));


    #50 // settle

    // Change data in - This should take 10 to propagate A->B but is instantaneous
    $display("change A - B will change immediately (but C is delayed as expected by 100)");

    Va=8'b00000000;

    timer=$time;
    nOE <= 0;
    wait(B === 8'b00000000);
    if ($time - timer < 10) begin
      $display("%6d", $time, " ERROR TOO QUICK - EXPECTED A->B = 10ns - BUT TOOK %-d", ($time - timer));
      failed = 1;
    end
    else
      $display("%6d", $time, " OK - TOOK %-d", ($time - timer));

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");

  end
endmodule
