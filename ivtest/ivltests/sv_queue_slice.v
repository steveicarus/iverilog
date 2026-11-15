// Slices of queues: Q[a:b] is a new queue with elements a through b.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$] = '{10, 20, 30, 40, 50};
  int r[$];
  int e[$];
  int a, b;
  integer ix;
  logic [7:0] v[$] = '{8'h11, 8'h22, 8'h33, 8'h44};
  logic [7:0] vr[$];
  real f[$] = '{1.5, 2.5, 3.5};
  real fr[$];
  string s[$] = '{"a", "bb", "ccc", "dddd"};
  string sr[$];
  int init_r[$] = q[1:2];
  int bq[$:1];
  logic [99:0] big;

  function automatic int sum(int x[$]);
    sum = 0;
    foreach (x[i]) sum += x[i];
  endfunction

  initial begin
    // Constant bounds.
    r = q[1:2];
    `CHK(r.size() === 2 && r[0] === 20 && r[1] === 30);
    `CHK(q.size() === 5);

    r = q[0:4];
    `CHK(r.size() === 5 && r[0] === 10 && r[4] === 50);

    // A single element.
    r = q[3:3];
    `CHK(r.size() === 1 && r[0] === 40);

    // Variable bounds.
    a = 2; b = 3;
    r = q[a:b];
    `CHK(r.size() === 2 && r[0] === 30 && r[1] === 40);
    r = q[a-1:b+1];
    `CHK(r.size() === 4 && r[0] === 20 && r[3] === 50);

    // $ is the last index.
    r = q[1:$];
    `CHK(r.size() === 4 && r[0] === 20 && r[3] === 50);
    r = q[0:$-1];
    `CHK(r.size() === 4 && r[0] === 10 && r[3] === 40);
    r = q[$-1:$];
    `CHK(r.size() === 2 && r[0] === 40 && r[1] === 50);

    // Empty when the lower bound is above the upper bound.
    r = q[3:1];
    `CHK(r.size() === 0);
    r = q[0:-1];
    `CHK(r.size() === 0);

    // The bounds are clamped to the queue.
    r = q[3:10];
    `CHK(r.size() === 2 && r[0] === 40 && r[1] === 50);
    r = q[-2:1];
    `CHK(r.size() === 2 && r[0] === 10 && r[1] === 20);
    r = q[-3:20];
    `CHK(r.size() === 5);
    r = q[7:9];
    `CHK(r.size() === 0);
    a = -1;
    r = q[a:a];
    `CHK(r.size() === 0);

    // An X bound gives an empty queue.
    ix = 'x;
    r = q[ix:2];
    `CHK(r.size() === 0);
    r = q[1:ix];
    `CHK(r.size() === 0);

    // Self assignment, the idiom for pop_front and pop_back.
    q = q[1:$];
    `CHK(q.size() === 4 && q[0] === 20 && q[3] === 50);
    q = q[0:$-1];
    `CHK(q.size() === 3 && q[0] === 20 && q[2] === 40);

    // A slice used as an initializer and as an argument.
    `CHK(init_r.size() === 2 && init_r[0] === 20 && init_r[1] === 30);
    `CHK(sum(q[1:2]) === 70);

    // Slices of an empty queue.
    r = e[0:3];
    `CHK(r.size() === 0);
    r = e[0:$];
    `CHK(r.size() === 0);
    q = '{};
    q = q[1:$];
    `CHK(q.size() === 0);

    // Other element types.
    vr = v[1:2];
    `CHK(vr.size() === 2 && vr[0] === 8'h22 && vr[1] === 8'h33);
    vr = v[2:$];
    `CHK(vr.size() === 2 && vr[0] === 8'h33 && vr[1] === 8'h44);

    fr = f[1:$];
    `CHK(fr.size() === 2 && fr[0] == 2.5 && fr[1] == 3.5);

    sr = s[1:2];
    `CHK(sr.size() === 2 && sr[0] == "bb" && sr[1] == "ccc");
    s = s[1:$];
    `CHK(s.size() === 3 && s[0] == "bb" && s[2] == "dddd");

    // The slice is a copy.
    q = '{1, 2, 3, 4};
    r = q[1:2];
    q[1] = 99;
    q.push_back(5);
    `CHK(r.size() === 2 && r[0] === 2 && r[1] === 3);
    q = '{1, 2, 3, 4};

    // A bound that is wider than 64 bits.
    big = 100'h1 << 80;
    r = q[0:big];
    `CHK(r.size() === 4);
    r = q[big:3];
    `CHK(r.size() === 0);
    big = -1;
    r = q[0:$signed(big)];
    `CHK(r.size() === 0);

    // A bounded queue gets the same elements as for a whole queue.
    bq = q[0:3];
    `CHK(bq.size() === 2 && bq[0] === 1 && bq[1] === 2);

    if (!failed)
      $display("PASSED");
  end
endmodule
