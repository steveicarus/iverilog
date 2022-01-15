module top;
  real q_tst [$];
  real q_tmp [$];
  real elem;
  integer idx;
  bit passed;

  task automatic check_size(integer size,
                            string fname,
                            integer lineno);
    if (q_tst.size !== size) begin
      $display("%s:%0d: Failed: queue size != %0d (%0d)",
               fname, lineno, size, q_tst.size);
      passed = 1'b0;
    end
  endtask

  task automatic check_idx_value(integer idx,
                                 real expected,
                                 string fname,
                                 integer lineno);
    if (q_tst[idx] != expected) begin
      $display("%s:%0d: Failed: element [%0d] != %.1f (%.1f)",
               fname, lineno, idx, expected, q_tst[idx]);
      passed = 1'b0;
    end
  endtask

  initial begin
    passed = 1'b1;

    q_tst.delete(0); // Warning: skip delete on an empty queue
    check_size(0, `__FILE__, `__LINE__);
    check_idx_value(0, 0.0, `__FILE__, `__LINE__);

    elem = q_tst.pop_front(); // Warning: cannot pop_front() an empty queue
    if (elem != 0.0) begin
      $display("Failed: pop_front() != 0.0 (%.1f)", elem);
      passed = 1'b0;
    end

    elem = q_tst.pop_back(); // Warning: cannot pop_back() an empty queue
    if (elem != 0.0) begin
      $display("Failed: pop_back() != 0.0 (%.1f)", elem);
      passed = 1'b0;
    end

    q_tst.push_back(2.0);
    q_tst.push_front(1.0);
    q_tst.push_back(3.0);
    q_tst.push_back(100.0);
    q_tst.delete(3); // Should $ work here?
    q_tst.delete(3); // Warning: skip an out of range delete()
    q_tst.delete(-1); // Warning: skip delete with negative index
    q_tst.delete('X); // Warning: skip delete with undefined index

    check_size(3, `__FILE__, `__LINE__);

    if (q_tst[0] != 1.0) begin
      $display("Failed: element [0] != 1.0 (%.1f)", q_tst[0]);
      passed = 1'b0;
    end

    if (q_tst[1] != 2.0) begin
      $display("Failed: element [1] != 2.0 (%.1f)", q_tst[1]);
      passed = 1'b0;
    end

    if (q_tst[2] != 3.0) begin
      $display("Failed: element [2] != 3.0 (%.1f)", q_tst[2]);
      passed = 1'b0;
    end

    if (q_tst[3] != 0.0) begin
      $display("Failed: element [3] != 0.0 (%.1f)", q_tst[3]);
      passed = 1'b0;
    end

    if (q_tst[-1] != 0.0) begin
      $display("Failed: element [-1] != 0.0 (%.1f)", q_tst[-1]);
      passed = 1'b0;
    end

    if (q_tst['X] != 0.0) begin
      $display("Failed: element ['X] != 0.0 (%.1f)", q_tst['X]);
      passed = 1'b0;
    end

    check_idx_value(-1, 0.0, `__FILE__, `__LINE__);
    check_idx_value('X, 0.0, `__FILE__, `__LINE__);

    elem = q_tst.pop_front();
    if (elem != 1.0) begin
      $display("Failed: element pop_front() != 1.0 (%.1f)", elem);
      passed = 1'b0;
    end

    elem = q_tst.pop_back();
    if (elem != 3.0) begin
      $display("Failed: element pop_back() != 3.0 (%.1f)", elem);
      passed = 1'b0;
    end

    check_size(1, `__FILE__, `__LINE__);

    if ((q_tst[0] != q_tst[$]) || (q_tst[0] != 2.0)) begin
      $display("Failed: q_tst[0](%.1f) != q_tst[$](%.1f) != 2.0",
               q_tst[0], q_tst[$]);
      passed = 1'b0;
    end

    q_tst.delete();

    check_size(0, `__FILE__, `__LINE__);

    q_tst.push_front(5.0);
    q_tst.push_front(100.0);
    q_tst.push_back(100.0);
    elem = q_tst.pop_back;
    elem = q_tst.pop_front;

    check_size(1, `__FILE__, `__LINE__);
    check_idx_value(0, 5.0, `__FILE__, `__LINE__);

    q_tst[0] = 1.0;
    q_tst[1] = 2.5;
    q_tst[1] = 2.0;
    q_tst[2] = 3.0;
    q_tst[-1] = 10.0; // Warning: will not be added (negative index)
    q_tst['X] = 10.0; // Warning: will not be added (undefined index)
    q_tst[4] = 10.0; // Warning: will not be added (out of range index)

    idx = -1;
    q_tst[idx] = 10.0; // Warning: will not be added (negative index)
    idx = 3'b0x1;
    q_tst[idx] = 10.0; // Warning: will not be added (undefined index)
    idx = 4;
    q_tst[idx] = 10.0; // Warning: will not be added (out of range index)

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 1.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);
    check_idx_value(2, 3.0, `__FILE__, `__LINE__);

    q_tst.delete();
    q_tst[0] = 2.0;
    q_tst.insert(1, 4.0);
    q_tst.insert(0, 1.0);
    q_tst.insert(2, 3.0);
    q_tst.insert(-1, 10.0); // Warning: will not be added (negative index)
    q_tst.insert('X, 10.0); // Warning: will not be added (undefined index)
    q_tst.insert(5, 10.0); // Warning: will not be added (out of range index)

    check_size(4, `__FILE__, `__LINE__);
    check_idx_value(0, 1.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);
    check_idx_value(2, 3.0, `__FILE__, `__LINE__);
    check_idx_value(3, 4.0, `__FILE__, `__LINE__);

    q_tst = '{3.0, 2.0, 1.0};

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 3.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);
    check_idx_value(2, 1.0, `__FILE__, `__LINE__);

    q_tmp = '{1.0, 2.0};
    q_tst = q_tmp;
    q_tmp[0] = 3.0;
    q_tmp[2] = 1.0;

    check_size(2, `__FILE__, `__LINE__);
    check_idx_value(0, 1.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);

    q_tst[2] = 3.0;
    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(2, 3.0, `__FILE__, `__LINE__);

    q_tst = {1.0, 2.0};
    check_size(2, `__FILE__, `__LINE__);
    check_idx_value(0, 1.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);

    q_tst = '{};

    check_size(0, `__FILE__, `__LINE__);

    if (passed) $display("PASSED");

   end
endmodule : top
