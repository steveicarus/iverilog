module top;
  reg pass;
  real rarr [1:0];
  reg [2:0] arr [1:0];
  integer index;
  integer base;
  reg [3*8:1] res;

  initial begin
    pass = 1'b1;
    rarr[0] = 1.0;
    rarr[1] = 2.0;
    arr[0] = 1;
    arr[1] = 2;
    base = 0;

    // Check an in range word select.
    index = 0;
    $sformat(res, "%3.1f", rarr[index+base]);
    if (res != "1.0") begin
      $display("Failed &A<> (real, index=0, thr.), expected 1.0, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3.1f", rarr[index]);
    if (res != "1.0") begin
      $display("Failed &A<> (real, index=0, sig.), expected 1.0, got %s", res);
      pass = 1'b0;
    end
    if (rarr[index] != 1.0) begin
      $display("Failed var select (real, index=0), expected 1.0");
      pass = 1'b0;
    end

    $sformat(res, "%3b", arr[index+base]);
    if (res != "001") begin
      $display("Failed &A<> (reg, index=0, thr.), expected 001, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3b", arr[index]);
    if (res != "001") begin
      $display("Failed &A<> (reg, index=0, sig.), expected 001, got %s", res);
      pass = 1'b0;
    end
    if (arr[index] != 3'b001) begin
      $display("Failed var select (reg, index=0), expected 3'b001");
      pass = 1'b0;
    end

    // Check an undefined array word select.
    index = 'bx;
    $sformat(res, "%3.1f", rarr[index+base]);
    if (res != "0.0") begin
      $display("Failed &A<> (real, index=x, thr.), expected 0.0, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3.1f", rarr[index]);
    if (res != "0.0") begin
      $display("Failed &A<> (real, index=x, sig.), expected 0.0, got %s", res);
      pass = 1'b0;
    end
    if (rarr[index] != 0.0) begin
      $display("Failed var select (real, index=x), expected 0.0");
      pass = 1'b0;
    end

    $sformat(res, "%3b", arr[index+base]);
    if (res != "xxx") begin
      $display("Failed &A<> (reg, index=x, thr.), expected xxx, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3b", arr[index]);
    if (res != "xxx") begin
      $display("Failed &A<> (reg, index=x, sig.), expected xxx, got %s", res);
      pass = 1'b0;
    end
    if (arr[index] != 3'bxxx) begin
      $display("Failed var select (reg, index=x), expected 3'bxxx");
      pass = 1'b0;
    end

    // Check a before the array word select.
    index = -1;
    $sformat(res, "%3.1f", rarr[index+base]);
    if (res != "0.0") begin
      $display("Failed &A<> (real, index=-1, thr.), expected 0.0, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3.1f", rarr[index]);
    if (res != "0.0") begin
      $display("Failed &A<> (real, index=-1, sig.), expected 0.0, got %s", res);
      pass = 1'b0;
    end
    if (rarr[index] != 0.0) begin
      $display("Failed var select (real, index=-1), expected 0.0");
      pass = 1'b0;
    end

    $sformat(res, "%3b", arr[index+base]);
    if (res != "xxx") begin
      $display("Failed &A<> (reg, index=-1, thr.), expected xxx, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3b", arr[index]);
    if (res != "xxx") begin
      $display("Failed &A<> (reg, index=-1, sig.), expected xxx, got %s", res);
      pass = 1'b0;
    end
    if (arr[index] != 3'bxxx) begin
      $display("Failed var select (reg, index=-1), expected 3'bxxx");
      pass = 1'b0;
    end

    // Check an after the array word select.
    index = 2;
    $sformat(res, "%3.1f", rarr[index+base]);
    if (res != "0.0") begin
      $display("Failed &A<> (real, index=2, thr.), expected 0.0, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3.1f", rarr[index]);
    if (res != "0.0") begin
      $display("Failed &A<> (real, index=2, sig.), expected 0.0, got %s", res);
      pass = 1'b0;
    end
    if (rarr[index] != 0.0) begin
      $display("Failed var select (real, index=2), expected 0.0");
      pass = 1'b0;
    end

    $sformat(res, "%3b", arr[index+base]);
    if (res != "xxx") begin
      $display("Failed &A<> (reg, index=2, thr.), expected xxx, got %s", res);
      pass = 1'b0;
    end
    $sformat(res, "%3b", arr[index]);
    if (res != "xxx") begin
      $display("Failed &A<> (reg, index=2, sig.), expected xxx, got %s", res);
      pass = 1'b0;
    end
    if (arr[index] != 3'bxxx) begin
      $display("Failed var select (reg, index=2), expected 3'bxxx");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
