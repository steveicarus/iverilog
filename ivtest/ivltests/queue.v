module top;
  reg pass;
  integer res, status, job, value;

  initial begin
    pass = 1'b1;
    // Use id = 1, type = 1 (FIFO) and a size of 3.
    $q_initialize(1, 1, 3, status);
    if (status !== 0) begin
      $display("Failed to initialize queue, got %d", status);
      pass = 1'b0;
    end

    // Use id = 2, type = 2 (LIFO) and a size of 2.
    $q_initialize(2, 2, 2, status);
    if (status !== 0) begin
      $display("Failed to initialize queue, got %d", status);
      pass = 1'b0;
    end

    // Use id = 3, type = 0 (undefined) and a size of 10.
    $q_initialize(3, 0, 10, status);
    if (status !== 4) begin
      $display("Failed to find invalid queue type (0), got %d", status);
      pass = 1'b0;
    end

    // Use id = 3, type = 3 (undefined) and a size of 10.
    $q_initialize(3, 3, 10, status);
    if (status !== 4) begin
      $display("Failed to find invalid queue type (3), got %d", status);
      pass = 1'b0;
    end

    // Use id = 3, type = 1 (FIFO) and a size of 0.
    $q_initialize(3, 1, 0, status);
    if (status !== 5) begin
      $display("Failed to find invalid queue size (0), got %d", status);
      pass = 1'b0;
    end

    // Use id = 3, type = 1 (FIFO) and a size of -1.
    $q_initialize(3, 1, -1, status);
    if (status !== 5) begin
      $display("Failed to find invalid queue size (-1), got %d", status);
      pass = 1'b0;
    end

    // This is a duplicate, so will fail.
    $q_initialize(1, 2, 20, status);
    if (status !== 6) begin
      $display("Failed to report duplicate queue, got %d", status);
      pass = 1'b0;
    end

    // Try to add to an invalid queue.
    $q_add(3, 0, 0, status);
    if (status !== 2) begin
      $display("Failed to report invalid queue ($q_add), got %d", status);
      pass = 1'b0;
    end

    // Try to remove from an invalid queue.
    $q_remove(3, job, value, status);
    if (status !== 2) begin
      $display("Failed to report invalid queue ($q_remove), got %d", status);
      pass = 1'b0;
    end

    // Try to check the status of an invalid queue.
    res = $q_full(3, status);
    if (status !== 2) begin
      $display("Failed to report invalid queue ($q_full), got %d", status);
      pass = 1'b0;
    end

    // Try to examine an invalid queue.
    $q_exam(3, 1, value, status);
    if (status !== 2) begin
      $display("Failed to report invalid queue ($q_exam), got %d", status);
      pass = 1'b0;
    end

    /*
     * Check the LIFO code.
     */

    // Add two element to the queue and check the queue state.
    $q_add(2, 1, 1, status);
    if (status !== 0) begin
      $display("Failed to add element 1 to the LIFO, got %d", status);
      pass = 1'b0;
    end
    res = $q_full(2, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (1), got %d (%d)", status, res);
      pass = 1'b0;
    end
    $q_add(2, 1, 2, status);
    if (status !== 0) begin
      $display("Failed to add element 2 to the LIFO, got %d", status);
      pass = 1'b0;
    end
    res = $q_full(2, status);
    if (status !== 0 || res !== 1) begin
      $display("Failed LIFO queue should be full, got %d (%d)", status, res);
      pass = 1'b0;
    end
    // Check some of the queue statistics.
    $q_exam(2, 1, value, status);
    if (status !== 0 || value !== 2) begin
      $display("Failed LIFO queue current length (full), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(2, 3, value, status);
    if (status !== 0 || value !== 2) begin
      $display("Failed LIFO queue maximum length (full), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(2, 5, value, status);
    if (status !== 0 || value !== 0) begin
      $display("Failed LIFO queue longest wait, got %d (%d)", status, value);
      pass = 1'b0;
    end

    // Adding a third element should return queue full.
    $q_add(2, 1, 3, status);
    if (status !== 1) begin
      $display("Failed to report the LIFO queue is full, got %d", status);
      pass = 1'b0;
    end

    // Now remove the elements from the queue.
    $q_remove(2, job, value, status);
    if (status !== 0 || job !== 1 || value !== 2) begin
      $display("Failed to remove element 2 from the LIFO, got %d (%d,%d)",
               status, job, value);
      pass = 1'b0;
    end
    res = $q_full(2, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (2), got %d (%d)", status, res);
      pass = 1'b0;
    end
    $q_remove(2, job, value, status);
    if (status !== 0 || job !== 1 || value !== 1) begin
      $display("Failed to remove element 1 from the LIFO, got %d (%d,%d)",
               status, job, value);
      pass = 1'b0;
    end
    res = $q_full(2, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (3), got %d (%d)", status, res);
      pass = 1'b0;
    end
    // Check some of the queue statistics.
    $q_exam(2, 1, value, status);
    if (status !== 0 || value !== 0) begin
      $display("Failed LIFO queue current length (empty), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(2, 3, value, status);
    if (status !== 0 || value !== 2) begin
      $display("Failed LIFO queue maximum length (empty), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(2, 4, value, status);
    if (status !== 0 || value !== 0) begin
      $display("Failed LIFO queue shortest wait, got %d (%d)", status, value);
      pass = 1'b0;
    end

    // Removing a third element should return queue empty.
    $q_remove(2, job, value, status);
    if (status !== 3) begin
      $display("Failed to report the LIFO queue is empty, got %d", status);
      pass = 1'b0;
    end

    /*
     * Check the FIFO code.
     */

    // Add three element to the queue and check the queue state.
    $q_add(1, 2, 1, status);
    if (status !== 0) begin
      $display("Failed to add element 1 to the FIFO, got %d", status);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (4), got %d (%d)", status, res);
      pass = 1'b0;
    end
    $q_add(1, 2, 2, status);
    if (status !== 0) begin
      $display("Failed to add element 2 to the FIFO, got %d", status);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (5), got %d (%d)", status, res);
      pass = 1'b0;
    end
    $q_add(1, 2, 3, status);
    if (status !== 0) begin
      $display("Failed to add element 3 to the FIFO, got %d", status);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 1) begin
      $display("Failed FIFO queue should be full, got %d (%d)", status, res);
      pass = 1'b0;
    end
    // Check some of the queue statistics.
    $q_exam(1, 1, value, status);
    if (status !== 0 || value !== 3) begin
      $display("Failed FIFO queue current length (full), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(1, 3, value, status);
    if (status !== 0 || value !== 3) begin
      $display("Failed FIFO queue maximum length (full), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(1, 5, value, status);
    if (status !== 0 || value !== 0) begin
      $display("Failed FIFO queue longest wait, got %d (%d)", status, value);
      pass = 1'b0;
    end

    // Adding a fourth element should return queue full.
    $q_add(1, 2, 4, status);
    if (status !== 1) begin
      $display("Failed to report the FIFO queue is full, got %d", status);
      pass = 1'b0;
    end

    // Now remove some of the elements from the queue.
    $q_remove(1, job, value, status);
    if (status !== 0 || job !== 2 || value !== 1) begin
      $display("Failed to remove element 1 from the FIFO, got %d (%d,%d)",
               status, job, value);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (6), got %d (%d)", status, res);
      pass = 1'b0;
    end
    $q_remove(1, job, value, status);
    if (status !== 0 || job !== 2 || value !== 2) begin
      $display("Failed to remove element 2 from the FIFO, got %d (%d,%d)",
               status, job, value);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (7), got %d (%d)", status, res);
      pass = 1'b0;
    end

    // Now add an element to wrap around the end.
    $q_add(1, 2, 4, status);
    if (status !== 0) begin
      $display("Failed to add element 4 to the FIFO, got %d", status);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (8), got %d (%d)", status, res);
      pass = 1'b0;
    end

    // Now empty the queue.
    $q_remove(1, job, value, status);
    if (status !== 0 || job !== 2 || value !== 3) begin
      $display("Failed to remove element 3 from the FIFO, got %d (%d,%d)",
               status, job, value);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (9), got %d (%d)", status, res);
      pass = 1'b0;
    end
    $q_remove(1, job, value, status);
    if (status !== 0 || job !== 2 || value !== 4) begin
      $display("Failed to remove element 2 from the FIFO, got %d (%d,%d)",
               status, job, value);
      pass = 1'b0;
    end
    res = $q_full(1, status);
    if (status !== 0 || res !== 0) begin
      $display("Failed queue should not be full (A), got %d (%d)", status, res);
      pass = 1'b0;
    end
    // Check some of the queue statistics.
    $q_exam(1, 1, value, status);
    if (status !== 0 || value !== 0) begin
      $display("Failed FIFO queue current length (empty), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(1, 3, value, status);
    if (status !== 0 || value !== 3) begin
      $display("Failed FIFO queue maximum length (empty), got %d (%d)",
               status, value);
      pass = 1'b0;
    end
    $q_exam(1, 4, value, status);
    if (status !== 0 || value !== 0) begin
      $display("Failed FIFO queue shortest wait, got %d (%d)", status, value);
      pass = 1'b0;
    end

    // Removing a fifth element should return queue empty.
    $q_remove(1, job, value, status);
    if (status !== 3) begin
      $display("Failed to report the FIFO queue is empty, got %d", status);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
