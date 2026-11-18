// $ is the index of the last element in the index of a queue.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$] = '{10, 20, 30, 40, 50};
  int r[$] = '{1, 2};

  initial begin
    `CHK(q[$] === 50);
    `CHK(q[$-1] === 40);
    `CHK(q[$-4] === 10);

    // A $ in the index of an index applies to the inner queue.
    `CHK(q[r[$]] === 30);
    `CHK(q[r[$-1]] === 20);

    // Overwrite the last element.
    q[$] = 55;
    `CHK(q.size() === 5 && q[4] === 55);
    q[$-1] = 44;
    `CHK(q.size() === 5 && q[3] === 44);

    // Writing one past the last element appends.
    q[$+1] = 60;
    `CHK(q.size() === 6 && q[5] === 60 && q[$] === 60);
    q[$+1] = 70;
    `CHK(q.size() === 7 && q[$] === 70);

    if (!failed)
      $display("PASSED");
  end
endmodule
