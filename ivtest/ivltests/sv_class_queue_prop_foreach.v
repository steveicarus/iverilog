// Regression: foreach over string and real queue and dynamic array class
// properties, inside methods.

module test;

  class C;
    string s[$];
    real r[$];
    string d[];
    real e[];
    int n;
    string cat;
    real sum;

    function void run();
      s.push_back("x"); s.push_back("y"); s.push_back("z");
      r.push_back(1.5); r.push_back(2.5);
      d = new[2]; d[0] = "p"; d[1] = "q";
      e = new[2]; e[0] = 0.5; e[1] = 0.25;

      foreach (s[k]) begin
        n += 1;
        cat = {cat, s[k]};
      end
      foreach (r[k]) sum += r[k];
      foreach (d[k]) cat = {cat, d[k]};
      foreach (e[k]) sum += e[k];
    endfunction
  endclass

  C c;

  initial begin
    c = new;
    c.run();
    if (c.n == 3 && c.cat == "xyzpq" && c.sum == 4.75)
      $display("PASSED");
    else
      $display("FAILED. n=%0d cat=%s sum=%f", c.n, c.cat, c.sum);
  end

endmodule
