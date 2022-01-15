
// This program should emit:
//  m_argv[0] = str0
//  LARGE: 4
//  LARGE: 4 (2)
//  m_argv[1] = str1
//  LARGE: 4
//  LARGE: 4 (2)
//

module m;

   string m_argv [$];

   function int size_function(input string val);
      size_function = val.len();
   endfunction // size_function

   initial begin
      m_argv.push_back ("str0");
      m_argv.push_back ("str1");

      foreach (m_argv[i]) begin
	 $display("m_argv[%0d] = %s", i, m_argv[i]);
	 if(m_argv[i].len() >= 2) begin
            $display ("LARGE: %0d", m_argv[i].len());
	 end else begin
	    $display("FAILED: m_argv[i].len() == %0d", m_argv[i].len());
	 end
	 if(size_function(m_argv[i]) >= 2) begin
            $display ("LARGE: %0d (2)", size_function(m_argv[i]));
	 end else begin
	    $display("FAILED: size_function(m_argv[i]) == %0d", size_function(m_argv[i]));
	 end
      end
   end
endmodule : m
