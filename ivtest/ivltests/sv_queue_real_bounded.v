module top;
  real q_tst [$:2];
  real q_tmp [$];
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

    check_size(0, `__FILE__, `__LINE__);

    q_tst.push_back(2.0);
    q_tst.push_front(1.0);
    q_tst.push_back(3.0);
    q_tst.push_back(100.0); // Warning: item not added.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 1.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);
    check_idx_value(2, 3.0, `__FILE__, `__LINE__);

    q_tst.push_front(0.5); // Warning: back item removed.
    q_tst[3] = 3.0; // Warning: item not added.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 0.5, `__FILE__, `__LINE__);
    check_idx_value(1, 1.0, `__FILE__, `__LINE__);
    check_idx_value(2, 2.0, `__FILE__, `__LINE__);

    q_tst.insert(3, 10.0); // Warning: item not added.
    q_tst.insert(1, 2.0); // Warning: back item removed.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 0.5, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);
    check_idx_value(2, 1.0, `__FILE__, `__LINE__);

    q_tst = '{1.0, 2.0, 3.0, 4.0}; // Warning: items not added.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 1.0, `__FILE__, `__LINE__);
    check_idx_value(1, 2.0, `__FILE__, `__LINE__);
    check_idx_value(2, 3.0, `__FILE__, `__LINE__);

    q_tmp = '{4.0, 3.0, 2.0, 1.0};
    q_tst = q_tmp; // Warning not all items copied
    q_tmp[0] = 5.0;

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, 4.0, `__FILE__, `__LINE__);
    check_idx_value(1, 3.0, `__FILE__, `__LINE__);
    check_idx_value(2, 2.0, `__FILE__, `__LINE__);

    if (passed) $display("PASSED");

   end
endmodule : top
