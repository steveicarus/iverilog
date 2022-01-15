module top;
  parameter weq1 = 2'b01 ==? 2'b01;
  parameter weq2 = 2'b01 ==? 2'b00;
  parameter weq3 = 2'b0x ==? 2'b00;
  parameter weq4 = 2'b00 ==? 2'b0x;
  parameter weq5 = 2'b01 ==? 2'b0x;
  parameter weq6 = 2'b0z ==? 2'b0x;
  parameter weq7 = 2'b0x ==? 2'b0x;
  parameter weq8 = 2'b00 ==? 2'b0z;
  parameter weq9 = 2'b01 ==? 2'b0z;
  parameter weqa = 2'b0z ==? 2'b0z;
  parameter weqb = 2'b0x ==? 2'b0z;
  parameter weqc = 2'bx0 ==? 2'b00;
  parameter weqd = 2'bx1 ==? 2'b00;
  parameter weqe = 2'b1x ==? 2'b00;
  parameter weqf = 3'b100 ==? 2'b00;
  parameter wneq1 = 2'b01 !=? 2'b01;
  parameter wneq2 = 2'b01 !=? 2'b00;
  parameter wneq3 = 2'b0x !=? 2'b00;
  parameter wneq4 = 2'b00 !=? 2'b0x;
  parameter wneq5 = 2'b01 !=? 2'b0x;
  parameter wneq6 = 2'b0z !=? 2'b0x;
  parameter wneq7 = 2'b0x !=? 2'b0x;
  parameter wneq8 = 2'b00 !=? 2'b0z;
  parameter wneq9 = 2'b01 !=? 2'b0z;
  parameter wneqa = 2'b0z !=? 2'b0z;
  parameter wneqb = 2'b0x !=? 2'b0z;
  parameter wneqc = 2'bx0 !=? 2'b00;
  parameter wneqd = 2'bx1 !=? 2'b00;
  parameter wneqe = 2'b1x !=? 2'b00;
  parameter wneqf = 3'b100 !=? 2'b00;

  reg pass;

  initial begin
    pass = 1'b1;

    if (weq1 !== 1'b1)  begin
      $display("Failed: parameter 2'b01 ==? 2'b01 returned 1'b%b not 1'b1", weq1);
      pass = 1'b0;
    end

    if (weq2 !== 1'b0)  begin
      $display("Failed: parameter 2'b01 ==? 2'b00 returned 1'b%b not 1'b0", weq2);
      pass = 1'b0;
    end

    if (weq3 !== 1'bx)  begin
      $display("Failed: parameter 2'b0x ==? 2'b00 returned 1'b%b not 1'bx", weq3);
      pass = 1'b0;
    end

    if (weq4 !== 1'b1)  begin
      $display("Failed: parameter 2'b00 ==? 2'b0x returned 1'b%b not 1'b1", weq4);
      pass = 1'b0;
    end

    if (weq5 !== 1'b1)  begin
      $display("Failed: parameter 2'b01 ==? 2'b0x returned 1'b%b not 1'b1", weq5);
      pass = 1'b0;
    end

    if (weq6 !== 1'b1)  begin
      $display("Failed: parameter 2'b0x ==? 2'b0x returned 1'b%b not 1'b1", weq6);
      pass = 1'b0;
    end

    if (weq7 !== 1'b1)  begin
      $display("Failed: parameter 2'b0z ==? 2'b0x returned 1'b%b not 1'b1", weq7);
      pass = 1'b0;
    end

    if (weq8 !== 1'b1)  begin
      $display("Failed: parameter 2'b00 ==? 2'b0z returned 1'b%b not 1'b1", weq8);
      pass = 1'b0;
    end

    if (weq9 !== 1'b1)  begin
      $display("Failed: parameter 2'b01 ==? 2'b0z returned 1'b%b not 1'b1", weq9);
      pass = 1'b0;
    end

    if (weqa !== 1'b1)  begin
      $display("Failed: parameter 2'b0x ==? 2'b0z returned 1'b%b not 1'b1", weqa);
      pass = 1'b0;
    end

    if (weqb !== 1'b1)  begin
      $display("Failed: parameter 2'b0z ==? 2'b0z returned 1'b%b not 1'b1", weqb);
      pass = 1'b0;
    end

    if (weqc !== 1'bx)  begin
      $display("Failed: parameter 2'bx0 ==? 2'b00 returned 1'b%b not 1'bx", weqc);
      pass = 1'b0;
    end

    if (weqd !== 1'b0)  begin
      $display("Failed: parameter 2'bx1 ==? 2'b00 returned 1'b%b not 1'b0", weqd);
      pass = 1'b0;
    end

    if (weqe !== 1'b0)  begin
      $display("Failed: parameter 2'b1x ==? 2'b00 returned 1'b%b not 1'b0", weqe);
      pass = 1'b0;
    end

    if (weqf !== 1'b0)  begin
      $display("Failed: parameter 3'b100 ==? 2'b00 returned 1'b%b not 1'b0", weqf);
      pass = 1'b0;
    end

    if (wneq1 !== 1'b0)  begin
      $display("Failed: parameter 2'b01 !=? 2'b01 returned 1'b%b not 1'b0", wneq1);
      pass = 1'b0;
    end

    if (wneq2 !== 1'b1)  begin
      $display("Failed: parameter 2'b01 !=? 2'b00 returned 1'b%b not 1'b1", wneq2);
      pass = 1'b0;
    end

    if (wneq3 !== 1'bx)  begin
      $display("Failed: parameter 2'b0x !=? 2'b00 returned 1'b%b not 1'bx", wneq3);
      pass = 1'b0;
    end

    if (wneq4 !== 1'b0)  begin
      $display("Failed: parameter 2'b00 !=? 2'b0x returned 1'b%b not 1'b0", wneq4);
      pass = 1'b0;
    end

    if (wneq5 !== 1'b0)  begin
      $display("Failed: parameter 2'b01 !=? 2'b0x returned 1'b%b not 1'b0", wneq5);
      pass = 1'b0;
    end

    if (wneq6 !== 1'b0)  begin
      $display("Failed: parameter 2'b0x !=? 2'b0x returned 1'b%b not 1'b0", wneq6);
      pass = 1'b0;
    end

    if (wneq7 !== 1'b0)  begin
      $display("Failed: parameter 2'b0z !=? 2'b0x returned 1'b%b not 1'b0", wneq7);
      pass = 1'b0;
    end

    if (wneq8 !== 1'b0)  begin
      $display("Failed: parameter 2'b00 !=? 2'b0z returned 1'b%b not 1'b0", wneq8);
      pass = 1'b0;
    end

    if (wneq9 !== 1'b0)  begin
      $display("Failed: parameter 2'b01 !=? 2'b0z returned 1'b%b not 1'b0", wneq9);
      pass = 1'b0;
    end

    if (wneqa !== 1'b0)  begin
      $display("Failed: parameter 2'b0x !=? 2'b0z returned 1'b%b not 1'b0", wneqa);
      pass = 1'b0;
    end

    if (wneqb !== 1'b0)  begin
      $display("Failed: parameter 2'b0z !=? 2'b0z returned 1'b%b not 1'b0", wneqb);
      pass = 1'b0;
    end

    if (wneqc !== 1'bx)  begin
      $display("Failed: parameter 2'bx0 !=? 2'b00 returned 1'b%b not 1'bx", wneqc);
      pass = 1'b0;
    end

    if (wneqd !== 1'b1)  begin
      $display("Failed: parameter 2'bx1 !=? 2'b00 returned 1'b%b not 1'b1", wneqd);
      pass = 1'b0;
    end

    if (wneqe !== 1'b1)  begin
      $display("Failed: parameter 2'b1x !=? 2'b00 returned 1'b%b not 1'b1", wneqe);
      pass = 1'b0;
    end

    if (wneqf !== 1'b1)  begin
      $display("Failed: parameter 3'b100 !=? 2'b00 returned 1'b%b not 1'b1", wneqf);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
