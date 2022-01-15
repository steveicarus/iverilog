
module main;

   string words [$], tmp_word;
   int	  nums [$], tmp_num;

   initial begin

      words.push_back("Hello");
      words.push_back("World");

      if (words.size != 2) begin
	 $display("FAILED -- words.size=%0d", words.size);
	 $finish;
      end

      if (words[0] != "Hello") begin
	 $display("FAILED -- words[0] = %s", words[0]);
	 $finish;
      end

      if (words[$] != "World") begin
	 $display("FAILED -- words[$] = %s", words[$]);
	 $finish;
      end

      tmp_word = words.pop_front();
      if (tmp_word != "Hello") begin
	 $display("FAILED -- words.pop_front()=%s", tmp_word);
	 $finish;
      end

      if (words[0] != words[$]) begin
	 $display("FAILED -- words[0](=%s) !== words[$](=%s)", words[0], words[$]);
	 $finish;
      end

      nums.push_back(2);
      nums.push_back(3);
      nums.push_front(1);

      if (nums.size != 3) begin
	 $display("FAILED -- nums.size=%0d", nums.size);
	 $finish;
      end

      if (nums[0] !== 1) begin
	 $display("FAILED -- nums[0] = %0d", nums[0]);
	 $finish;
      end

      if (nums[$] !== 3) begin
	 $display("FAILED -- nums[$] = %0d", nums[$]);
	 $finish;
      end

      tmp_num = nums.pop_back();
      if (tmp_num !== 3) begin
	 $display("FAILED -- tmp_num=%0d (from back)", tmp_num);
	 $finish;
      end

      if (nums.size !== 2) begin
	 $display("FAILED -- nums.size after pop_back = %0d", nums.size);
	 $finish;
      end

      if (nums[0] !== 1) begin
	 $display("FAILED -- nums[0] = %0d", nums[0]);
	 $finish;
      end

      if (nums[1] !== 2) begin
	 $display("FAILED -- nums[1] = %0d", nums[1]);
	 $finish;
      end

      tmp_num = nums.pop_front();
      if (tmp_num !== 1) begin
	 $display("FAILED == tmp_num=%0d (fron front)", tmp_num);
	 $finish;
      end
      if (nums.size !== 1) begin
	 $display("FAILED -- nums.size after pop_front = %0d", nums.size);
	 $finish;
      end

      if (nums[0] !== 2) begin
	 $display("FAILED -- nums[0] = %0d after pop_front", nums[0]);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
