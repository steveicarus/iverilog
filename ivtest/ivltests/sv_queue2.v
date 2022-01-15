
module main;

   string words [$];
   int	  nums [$];

   initial begin

      words.push_back("Hello");
      words.push_back("World");

      nums.push_back(1);
      nums.push_back(2);
      nums.push_front(0);

      foreach (words[widx]) begin
	 case (widx)
	   0: if (words[widx] != "Hello") begin
	      $display("FAILED -- words[%0d] == %s", widx, words[widx]);
	      $finish;
	   end

	   1: if (words[widx] != "World") begin
	      $display("FAILED -- words[%0d] == %s", widx, words[widx]);
	      $finish;
	   end

	   default:
	     begin
		$display("FAILED -- widx = %0d", widx);
		$finish;
	     end
	   endcase
      end

      foreach (nums[nidx]) begin
	 if (nidx !== nums[nidx]) begin
	    $display("FAILED -- nums[%0d] == %0d", nidx, nums[nidx]);
	    $finish;
	 end
      end

      $display("PASSED");
   end
endmodule // main
