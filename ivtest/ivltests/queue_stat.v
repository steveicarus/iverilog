/*
 * I expect the following statistics results:
 *   0 x 0 x x x
 *   1 x 1 x 0 0
 *   2 1 2 x 1 0
 *   3 1 3 x 2 1
 *   2 1 3 3 2 2
 *   2 1 3 3 3 2
 *   3 1 3 3 4 2
 *   1 1 3 3 1 3
 *   0 1 3 2 x 3
 *   1 2 3 2 0 2
 *   0 2 3 1 x 3
 *   1 3 3 1 0 2
 *   2 2 3 1 1 2
 *   3 2 3 1 2 2
 *
 * x implies there is no defined value for that statistic.
 */
module top;
  reg pass;
  integer res, status, value, value2;
  integer id, job, item;

  initial begin
    pass = 1'b1;

    id = 1;
    // Use id = 1, type = 1 (FIFO) and a size of 5.
    $display("----- INIT -----");
    $q_initialize(id, 1, 5, status);
    if (status !== 0) begin
      $display("Failed to initialize queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    // Add an element to the queue.
    job = 1;
    item = 10;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Add a second element to the queue.
    job = 1;
    item = 20;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Add a third element to the queue.
    job = 1;
    item = 30;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Remove an element from the queue.
    $display("----- REMOVE -----");
    $q_remove(id, value, value2, status);
    if (status !== 0) begin
      $display("Failed to remove element from the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1
    print_stats(id);

    #1;
    // Add a fourth element to the queue.
    job = 1;
    item = 30;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Remove two elements from the queue.
    $display("----- REMOVE TWO -----");
    $q_remove(id, value, value2, status);
    if (status !== 0) begin
      $display("Failed to remove element (1) from the queue, got %d", status);
      pass = 1'b0;
    end
    $q_remove(id, value, value2, status);
    if (status !== 0) begin
      $display("Failed to remove element (2) from the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Remove an element from the queue.
    $display("----- REMOVE -----");
    $q_remove(id, value, value2, status);
    if (status !== 0) begin
      $display("Failed to remove element from the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Add a fifth element to the queue.
    job = 1;
    item = 50;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Remove an element from the queue.
    $display("----- REMOVE -----");
    $q_remove(id, value, value2, status);
    if (status !== 0) begin
      $display("Failed to remove element from the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #7;
    // Add a sixth element to the queue.
    job = 1;
    item = 60;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Add a seventh element to the queue.
    job = 1;
    item = 70;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    #1;
    // Add a eight element to the queue.
    job = 1;
    item = 80;
    $display("----- ADD -----");
    $q_add(id, job, item, status);
    if (status !== 0) begin
      $display("Failed to add element to the queue, got %d", status);
      pass = 1'b0;
    end
    print_stats(id);

    if (pass) $display("PASSED");
  end

  task print_stats;
    input id;
    integer id;
    integer len, inter, max, short, long, avg, status;

    // Icarus uses a status code of 10 to indicate no statistics available.
    begin
      len = 32'bx;
      inter = 32'bx;
      max = 32'bx;
      short = 32'bx;
      long = 32'bx;
      avg = 32'bx;
      $display("Queue statistics at time %0d", $time);
      // Get the queue length.
      $q_exam(id, 1, len, status);
      if ((status !== 0) && (status !== 10)) begin
        $display("  Failed to get the length, status %0d", status);
        pass = 1'b0;
      end
      // Get the mean inter-arrival time.
      $q_exam(id, 2, inter, status);
      if ((status !== 0) && (status !== 10)) begin
        $display("  Failed to get the inter-arrival, status %0d", status);
        pass = 1'b0;
      end
      // Get the maximum length.
      $q_exam(id, 3, max, status);
      if ((status !== 0) && (status !== 10)) begin
	$display("  Failed to get the maximum length, status %0d", status);
	pass = 1'b0;
      end
      // Get the shortest wait time.
      $q_exam(id, 4, short, status);
      if ((status !== 0) && (status !== 10)) begin
	$display("  Failed to get the shortest wait time, status %0d", status);
	pass = 1'b0;
      end
      // Get the longest wait time.
      $q_exam(id, 5, long, status);
      if ((status !== 0) && (status !== 10)) begin
	$display("  Failed to get the longest wait time, status %0d", status);
	pass = 1'b0;
      end
      // Get the average wait time.
      $q_exam(id, 6, avg, status);
      if ((status !== 0) && (status !== 10)) begin
	$display("  Failed to get the average wait time, status %0d", status);
	pass = 1'b0;
      end
      $display("    %0d, %0d, %0d, %0d, %0d, %0d",
               len, inter, max, short, long, avg);
    end
  endtask
endmodule
