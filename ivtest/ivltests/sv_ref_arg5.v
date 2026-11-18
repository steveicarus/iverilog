// Ref arguments of automatic functions, called from expressions, and
// of the different data types.

class item;
  int id;
endclass

module top;
  int total;
  int count;

  function automatic int accumulate(ref int sum, input int v);
    sum = sum + v;
    return sum;
  endfunction

  function automatic void inc(ref int x);
    x = x + 1;
  endfunction

  function automatic real halve(ref real r);
    r = r / 2.0;
    return r;
  endfunction

  function automatic string append(ref string s, input string t);
    s = {s, t};
    return s;
  endfunction

  function automatic void set_item(ref item it, input int id);
    it = new;
    it.id = id;
  endfunction

  function automatic void push(ref int q[$], input int v);
    q.push_back(v);
  endfunction

  function automatic void grow(ref int d[], input int n);
    d = new[n];
    foreach (d[i]) d[i] = i * 3;
  endfunction

  // A ref argument that is passed on to another subroutine.
  function automatic void inc_twice(ref int x);
    inc(x);
    inc(x);
  endfunction

  task automatic bump_all(ref int x, ref int y);
    inc_twice(x);
    inc_twice(y);
    inc(x);
  endtask

  // An automatic subroutine can pass its own variables.
  function automatic int local_var();
    int l;
    l = 10;
    inc(l);
    inc_twice(l);
    return l;
  endfunction

  initial begin
    real r;
    string s;
    item it;
    int q[$];
    int d[];
    int x, y, res;

    total = 0;
    res = accumulate(total, 5) + accumulate(total, 6);
    // The second call sees the write of the first: 5 + 11.
    if (total !== 11 || res !== 16) begin
      $display("FAILED: total=%0d res=%0d (expect 11, 16)", total, res);
      $finish;
    end

    count = 0;
    inc(count);
    inc(count);
    if (count !== 2) begin
      $display("FAILED: count=%0d (expect 2)", count);
      $finish;
    end

    r = 5.0;
    if (halve(r) != 2.5 || r != 2.5) begin
      $display("FAILED: r=%f (expect 2.5)", r);
      $finish;
    end

    s = "ab";
    if (append(s, "cd") != "abcd" || s != "abcd") begin
      $display("FAILED: s=%s (expect abcd)", s);
      $finish;
    end

    set_item(it, 17);
    if (it == null || it.id !== 17) begin
      $display("FAILED: item not set");
      $finish;
    end

    push(q, 4);
    push(q, 5);
    if (q.size() !== 2 || q[0] !== 4 || q[1] !== 5) begin
      $display("FAILED: queue size=%0d", q.size());
      $finish;
    end

    grow(d, 4);
    if (d.size() !== 4 || d[3] !== 9) begin
      $display("FAILED: darray size=%0d", d.size());
      $finish;
    end

    x = 0;
    y = 100;
    bump_all(x, y);
    if (x !== 3 || y !== 102) begin
      $display("FAILED: x=%0d y=%0d (expect 3, 102)", x, y);
      $finish;
    end

    if (local_var() !== 13) begin
      $display("FAILED: local_var()=%0d (expect 13)", local_var());
      $finish;
    end

    $display("PASSED");
  end
endmodule
