// Associative-array vertical slice smoke test (string keys).
// Also accepts int aa[*] as string-keyed AA in this first slice.
module aa_string;
  int aa[string];
  int bb[string];
  int aa_star[*];  // documented: [*] is string-keyed in this slice
  int v, n, e;
  int pass;
  string k;
  int sum;

  initial begin
    pass = 1;

    aa["x"] = 1;
    aa["y"] = 2;
    aa["z"] = 3;
    v = aa["x"];
    if (v !== 1) begin
      $display("FAIL: aa[\"x\"] read got %0d", v);
      pass = 0;
    end

    n = aa.size();
    if (n !== 3) begin
      $display("FAIL: size() expected 3 got %0d", n);
      pass = 0;
    end
    n = aa.num();
    if (n !== 3) begin
      $display("FAIL: num() expected 3 got %0d", n);
      pass = 0;
    end

    e = aa.exists("x");
    if (e !== 1) begin
      $display("FAIL: exists(\"x\") expected 1 got %0d", e);
      pass = 0;
    end
    e = aa.exists("missing");
    if (e !== 0) begin
      $display("FAIL: exists(\"missing\") expected 0 got %0d", e);
      pass = 0;
    end

    sum = 0;
    foreach (aa[k]) begin
      sum = sum + aa[k];
    end
    if (sum !== 6) begin
      $display("FAIL: foreach sum expected 6 got %0d", sum);
      pass = 0;
    end

    bb = aa;
    if (bb.size() !== 3 || bb["y"] !== 2) begin
      $display("FAIL: whole-array copy");
      pass = 0;
    end

    aa.delete("x");
    if (aa.exists("x") !== 0 || aa.size() !== 2) begin
      $display("FAIL: delete(\"x\")");
      pass = 0;
    end

    aa.delete();
    if (aa.size() !== 0) begin
      $display("FAIL: delete() clear all, size=%0d", aa.size());
      pass = 0;
    end

    aa_star["hello"] = 42;
    if (aa_star["hello"] !== 42 || aa_star.size() !== 1) begin
      $display("FAIL: [*] string-keyed AA");
      pass = 0;
    end

    if (pass) $display("PASSED");
    else $display("FAILED");
    $finish;
  end
endmodule
