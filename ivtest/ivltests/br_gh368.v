module top;

task test_task;
  begin
    fork
      begin
	$display("Process #1");
	#20
	$display("Process #1 -- completes");
      end
      begin
	// Here's a timeout task.  It should only complete if
	// something in process #1 fails to complete.  As such,
	// it operates as a failsafe.
	#1
        $display("Process #2");
	#200
	$display("Process #2 -- completes");
      end
    join_any;
    disable fork;

    $display("Test task completes");
  end
endtask

initial	test_task;

endmodule
