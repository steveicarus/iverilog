module top;
  parameter le0 = 1'b0 <-> 1'b0; // 1'b1
  parameter le1 = 1'b0 <-> 1'b1; // 1'b0
  parameter le2 = 1'b0 <-> 1'bz; // 1'bx
  parameter le3 = 1'b0 <-> 1'bx; // 1'bx
  parameter le4 = 1'b1 <-> 1'b0; // 1'b0
  parameter le5 = 1'b1 <-> 1'b1; // 1'b1
  parameter le6 = 1'b1 <-> 1'bz; // 1'bx
  parameter le7 = 1'b1 <-> 1'bx; // 1'bx
  parameter le8 = 1'bz <-> 1'b0; // 1'bx
  parameter le9 = 1'bz <-> 1'b1; // 1'bx
  parameter lea = 1'bz <-> 1'bz; // 1'bx
  parameter leb = 1'bz <-> 1'bx; // 1'bx
  parameter lec = 1'bx <-> 1'b0; // 1'bx
  parameter led = 1'bx <-> 1'b1; // 1'bx
  parameter lee = 1'bx <-> 1'bz; // 1'bx
  parameter lef = 1'bx <-> 1'bx; // 1'bx

  parameter [1:0] lew = 4'b0110 <-> 4'b1001; // 2'b01
  parameter [1:0] lews = $signed(4'b0110 <-> 4'b1001); // 2'b11
  parameter ler0 = 0.0 <-> 1'b0; // 1'b1
  parameter ler1 = 1'b0 <-> 2.0; // 1'b0
  parameter ler2 = 2.0 <-> 1'bx; // 1'bx
  parameter ler3 = -5.0 <-> 2.0; // 1'b1

  reg pass;

  initial begin
    pass = 1'b1;

    if (le0 !== 1'b1) begin
      $display("FAILED: 1'b0 <-> 1'b0 returned %b not 1'b1", le0);
      pass = 1'b0;
    end
    if (le1 !== 1'b0) begin
      $display("FAILED: 1'b0 <-> 1'b1 returned %b not 1'b0", le1);
      pass = 1'b0;
    end
    if (le2 !== 1'bx) begin
      $display("FAILED: 1'b0 <-> 1'bz returned %b not 1'bx", le2);
      pass = 1'b0;
    end
    if (le3 !== 1'bx) begin
      $display("FAILED: 1'b0 <-> 1'bx returned %b not 1'bx", le3);
      pass = 1'b0;
    end
    if (le4 !== 1'b0) begin
      $display("FAILED: 1'b1 <-> 1'b0 returned %b not 1'b0", le4);
      pass = 1'b0;
    end
    if (le5 !== 1'b1) begin
      $display("FAILED: 1'b1 <-> 1'b1 returned %b not 1'b1", le5);
      pass = 1'b0;
    end
    if (le6 !== 1'bx) begin
      $display("FAILED: 1'b1 <-> 1'bz returned %b not 1'bx", le6);
      pass = 1'b0;
    end
    if (le7 !== 1'bx) begin
      $display("FAILED: 1'b1 <-> 1'bx returned %b not 1'bx", le7);
      pass = 1'b0;
    end
    if (le8 !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'b0 returned %b not 1'bx", le8);
      pass = 1'b0;
    end
    if (le9 !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'b1 returned %b not 1'bx", le9);
      pass = 1'b0;
    end
    if (lea !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'bz returned %b not 1'bx", lea);
      pass = 1'b0;
    end
    if (leb !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'bx returned %b not 1'bx", leb);
      pass = 1'b0;
    end
    if (lec !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'b0 returned %b not 1'bx", lec);
      pass = 1'b0;
    end
    if (led !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'b0 returned %b not 1'bx", led);
      pass = 1'b0;
    end
    if (lee !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'bz returned %b not 1'bx", lee);
      pass = 1'b0;
    end
    if (lef !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'bx returned %b not 1'bx", lef);
      pass = 1'b0;
    end

    if (ler0 !== 1'b1) begin
      $display("FAILED: 0.0 <-> 1'b0 returned %b not 1'b1", ler0);
      pass = 1'b0;
    end
    if (ler1 !== 1'b0) begin
      $display("FAILED: 1'b0 <-> 2.0 returned %b not 1'b1", ler1);
      pass = 1'b0;
    end
    if (ler2 !== 1'bx) begin
      $display("FAILED: 2.0 <-> 1'bx returned %b not 1'b1", ler2);
      pass = 1'b0;
    end
    if (ler3 !== 1'b1) begin
      $display("FAILED: -5.0 <-> 2.0 returned %b not 1'b1", ler3);
      pass = 1'b0;
    end
    if (lew !== 2'b01) begin
      $display("FAILED: 4'b0110 <-> 4'b1001 returned %b not 2'b01", lew);
      pass = 1'b0;
    end
    if (lews !== 2'b11) begin
      $display("FAILED: 4'b0110 <-> 4'b1001 returned %b not 2'b11", lews);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
