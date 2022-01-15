module top;
  string q_tst [$];
  string q_tmp [$];
  string elem;
  integer idx;
  bit passed;

  task automatic check_size(integer size,
                            string fname,
                            integer lineno);
    if (q_tst.size !== size) begin
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

    q_tst.delete(0); // Warning: skip delete on an empty queue
    check_size(0, `__FILE__, `__LINE__);
    check_idx_value(0, "", `__FILE__, `__LINE__);

    elem = q_tst.pop_front(); // Warning: cannot pop_front() an empty queue
    if (elem != "") begin
      $display("Failed: pop_front() != '' ('%s')", elem);
      passed = 1'b0;
    end

    elem = q_tst.pop_back(); // Warning: cannot pop_back() an empty queue
    if (elem != "") begin
      $display("Failed: pop_back() != '' ('%s')", elem);
      passed = 1'b0;
    end

    q_tst.push_back("World");
    q_tst.push_front("Hello");
    q_tst.push_back("!");
    q_tst.push_back("This should get deleted");
    q_tst.delete(3);
    q_tst.delete(3); // Warning: skip an out of range delete()
    q_tst.delete(-1); // Warning: skip delete with negative index
    q_tst.delete('X); // Warning: skip delete with undefined index

    check_size(3, `__FILE__, `__LINE__);

    if (q_tst[0] != "Hello") begin
      $display("Failed: element [0] != 'Hello' ('%s')", q_tst[0]);
      passed = 1'b0;
    end

    if (q_tst[1] != "World") begin
      $display("Failed: element [1] != 'World' ('%s')", q_tst[1]);
      passed = 1'b0;
    end

    if (q_tst[2] != "!") begin
      $display("Failed: element [2] != '!' ('%s')", q_tst[2]);
      passed = 1'b0;
    end

    if (q_tst[3] != "") begin
      $display("Failed: element [3] != '' ('%s')", q_tst[3]);
      passed = 1'b0;
    end

    if (q_tst[-1] != "") begin
      $display("Failed: element [-1] != '' ('%s')", q_tst[-1]);
      passed = 1'b0;
    end

    if (q_tst['X] != "") begin
      $display("Failed: element ['X] != '' ('%s')", q_tst['X]);
      passed = 1'b0;
    end

    check_idx_value(-1, "", `__FILE__, `__LINE__);
    check_idx_value('X, "", `__FILE__, `__LINE__);

    elem = q_tst.pop_front();
    if (elem != "Hello") begin
      $display("Failed: element pop_front() != 'Hello' ('%s')", elem);
      passed = 1'b0;
    end

    elem = q_tst.pop_back();
    if (elem != "!") begin
      $display("Failed: element pop_back() != '!' ('%s')", elem);
      passed = 1'b0;
    end

    check_size(1, `__FILE__, `__LINE__);

    if ((q_tst[0] != q_tst[$]) || (q_tst[0] != "World")) begin
      $display("Failed: q_tst[0]('%s') != q_tst[$]('%s') != 'World'",
               q_tst[0], q_tst[$]);
      passed = 1'b0;
    end

    q_tst.delete();

    check_size(0, `__FILE__, `__LINE__);

    q_tst.push_front("hello");
    q_tst.push_front("Will be removed");
    q_tst.push_back("Will also be removed");
    elem = q_tst.pop_back;
    elem = q_tst.pop_front;

    check_size(1, `__FILE__, `__LINE__);
    check_idx_value(0, "hello", `__FILE__, `__LINE__);

    q_tst[0] = "Hello";
    q_tst[1] = "world";
    q_tst[1] = "World";
    q_tst[2] = "!";
    q_tst[-1] = "Will not write"; // Warning: will not be added (negative index)
    q_tst['X] = "Will not write"; // Warning: will not be added (undefined index)
    q_tst[4] = "Will not write"; // Warning: will not be added (out of range index)

    idx = -1;
    q_tst[idx] = "Will not write"; // Warning: will not be added (negative index)
    idx = 3'b0x1;
    q_tst[idx] = "Will not write"; // Warning: will not be added (undefined index)
    idx = 4;
    q_tst[idx] = "Will not write"; // Warning: will not be added (out of range index)

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "Hello", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);
    check_idx_value(2, "!", `__FILE__, `__LINE__);

    q_tst.delete();
    q_tst[0] = "World";
    q_tst.insert(1, "Again");
    q_tst.insert(0, "Hello");
    q_tst.insert(2, "!");
    q_tst.insert(-1, "Will not be added"); // Warning: will not be added (negative index)
    q_tst.insert('X, "Will not be added"); // Warning: will not be added (undefined index)
    q_tst.insert(5, "Will not be added"); // Warning: will not be added (out of range index)

    check_size(4, `__FILE__, `__LINE__);
    check_idx_value(0, "Hello", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);
    check_idx_value(2, "!", `__FILE__, `__LINE__);
    check_idx_value(3, "Again", `__FILE__, `__LINE__);

    q_tst = '{"!", "World", "Hello"};

    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(0, "!", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);
    check_idx_value(2, "Hello", `__FILE__, `__LINE__);

    q_tmp = '{"Hello", "World"};
    q_tst = q_tmp;
    q_tmp[0] = "Goodbye";
    q_tmp[2] = "Not seen";

    check_size(2, `__FILE__, `__LINE__);
    check_idx_value(0, "Hello", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);

    q_tst[2] = "Added, but removed";
    check_size(3, `__FILE__, `__LINE__);
    check_idx_value(2, "Added, but removed", `__FILE__, `__LINE__);

    q_tst = {"Hello", "World"};
    check_size(2, `__FILE__, `__LINE__);
    check_idx_value(0, "Hello", `__FILE__, `__LINE__);
    check_idx_value(1, "World", `__FILE__, `__LINE__);

    q_tst = '{};

    check_size(0, `__FILE__, `__LINE__);

    if (passed) $display("PASSED");

   end
endmodule : top
