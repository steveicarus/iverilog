module top;
  string q_tst [$:2];
  string q_tmp [$];
  bit passed;

  task automatic check_size(integer size,
                            string fname,
                            integer lineno);
    if (q_tst.size() !== size) begin
      $display("%s:%0d: Failed: queue initial size != %0d (%0d)",
               fname, lineno, size, q_tst.size);
      passed = 1'b0;
    end
  endtask

  task automatic check_idx_value(integer idx,
                                 string expected,
                                 string fname,
                                 integer lineno);
    if (q_tst[idx] != expected) begin
      $display("%s:%0d: Failed: element [%0d] != '%s' ('%s')",
               fname, lineno, idx, expected, q_tst[idx]);
      passed = 1'b0;
    end
  endtask

  initial begin
    passed = 1'b1;

    check_size(0, `__FILE__, `__LINE__);

    q_tst.push_back("World");
    q_tst.push_front("Hello");
    q_tst.push_back("!");
    q_tst.push_back("This will not be added"); // Warning: item not added.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "Hello", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);
    check_idx_value(2, "!", `__FILE__, `__LINE__);

    q_tst.push_front("I say,"); // Warning: sback item removed.
    q_tst[3] = "Will not be added"; // Warning: item not added.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "I say,", `__FILE__, `__LINE__);
    check_idx_value(1, "Hello", `__FILE__, `__LINE__);
    check_idx_value(2, "World", `__FILE__, `__LINE__);

    q_tst.insert(3, "Will not be added"); // Warning: item not added.
    q_tst.insert(1, "to you"); // Warning: back item removed.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "I say,", `__FILE__, `__LINE__);
    check_idx_value(1, "to you", `__FILE__, `__LINE__);
    check_idx_value(2, "Hello", `__FILE__, `__LINE__);

    q_tst = '{"Hello", "World", "!", "Will not be added"}; // Warning: items not added.

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "Hello", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);
    check_idx_value(2, "!", `__FILE__, `__LINE__);


    q_tmp = '{"Again,", "Hello", "World", "!"};
    q_tst = q_tmp; // Warning not all items copied
    q_tmp[0] = "Will not change anything";

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "Again,", `__FILE__, `__LINE__);
    check_idx_value(1, "Hello", `__FILE__, `__LINE__);
    check_idx_value(2, "World", `__FILE__, `__LINE__);

    if (passed) $display("PASSED");

   end
endmodule : top
