// Eleven basic tests in here:
// 1. longint must be initialised before any initial or always block
// 2. assignments to (signed) longint with random numbers
// 3. assignments to (signed) longint with random values including X and Z
// 4. converting unsigned 64-bit integer time to signed longint
// 5. converting signed integers to signed longint
// 6. converting 64-bit integers including X and Z states to unsigned longint
// 7. trying unsigned sums (procedural, function, task and module)
// 8. trying unsigned mults (procedural, function and task)
// 9. trying relational operators
// 10. smaller signed numbers to unsigned longint (signed extension)
// 11. trying some concatenations from bytes, shortints, ints to longints

module ms_add (input longint signed a, b, output longint signed sc, ss);
   assign sc = a + b;
   always @(a, b) ss = a + b;
endmodule

module main;
  parameter N_REPS = 500;               // repetition with random numbers
  parameter XZ_REPS = 500;              // repetition with 'x 'z values
  parameter MAX8 = 'h7f;
  parameter MAX16 = 'h7fff;
  parameter LEN = 64;
  // variables used as golden references
  reg signed [LEN-1:0] ar;              // holds numbers
  reg signed [LEN-1:0] ar_xz;           // holds 'x and/or 'z in random positions
  reg signed [LEN-1:0] ar_expected;
  reg unsigned [LEN-1:0]   ui;                          // unsigned 64-bit integer
  reg signed   [LEN-1:0]   si;                          // signed 64-bit integer
  reg signed [LEN/2-1:0]       slice;

  // type assumed to be tested before hand
  byte     signed pt1, pt2;
  shortint signed ps1, ps2;
  int      signed pv1, pv2;

  // types to be tested
  longint signed bu;                       // holds numbers
  longint signed bu_xz;                    // 'x and 'z are attempted on this
  longint signed bresult;                  // hold results from sums and mults
  longint signed mcaresult;                // wired to a module instance
  longint signed mabresult;                // also wired to a module instance


  integer i;

  // continuous assigments
  // type LHS      type RHS
  // ---------     ---------
  // longint       4-value logic
  assign bu = ar;
  assign bu_xz = ar_xz;

  // module instantiation
  ms_add duv (.a(bu), .b(bu_xz), .sc(mcaresult), .ss(mabresult) );

  // all test
  initial begin
    // time 0 checkings (Section 6.4 of IEEE 1850 LRM)
    if (bu !== 64'b0 || bu_xz != 64'b0 || bresult !== 64'b0 || mcaresult !== 64'b0 || mabresult !== 64'b0)
      begin
        $display ("FAILED - time zero initialisation incorrect: %b %b", bu, bu_xz);
        $finish;
      end
    // driving longint type with signed random numbers from a variable
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = { $random, $random };
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect assigment to int: %b", bu);
            $finish;
        end
      end
    # 1;
    // attempting to drive variables having 'x 'z values into type unsigned longint
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        ar = { $random, $random };
        ar_xz = xz_inject (ar);
        ar_expected = xz_expected (ar_xz);
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect assigment to longint (when 'x 'z): %b", bu);
            $finish;
        end
      end
    // converting unsigned 64-bit integers (time) to unsigned longint
    // this should pass trivially
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ui = { {$random}, {$random} };
        #1;
        force bu = ui;
        #1;
        if (bu !== ui)
          begin
            $display ("FAILED - incorrect assignment from 64-bit integer to longint: %b", bu);
            $finish;
        end
      end
    release bu;
    // converting signed 64-bit integers to unsigned longints
    // keeping the same bit representation is expected
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        si = { {$random}, {$random} };
        #1;
        force bu = si;
        #1;
        if (bu !== si)
          begin
            $display ("FAILED - incorrect assignment from 64-bit signed integer to longint: %d mismatchs %d", bu, -ui);
            $finish;
        end
      end
    release bu;
    // converting integers having 'x 'z values into type unsigned longint
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    // coercion to zero expected
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        ui = { {$random}, {$random} };
        ar_xz = xz_inject (ui);
        ui = ar_xz;
        ar_expected = xz_expected (ar_xz);
        #1;
        force bu_xz = ui;
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect conversion from 64-bit integer (with 'x 'z) to longint: %b mismatchs %b", bu_xz, ar_expected);
            $finish;
        end
      end
    release bu_xz;
    // trying signed sums
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = { $random, $random };
        ar_xz = { $random, $random };
        #1;
        bresult = bu + bu_xz;
        #1;
        if ( bresult !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed longints: %0d mismatchs %0d", bresult, s_sum(ar, ar_xz));
            $finish;
          end
        // invoking longint sum function
        if ( fs_sum (bu, bu_xz) !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of unsigned longint in function");
            $finish;
          end
        // invoking longint sum task
        ts_sum (bu, bu_xz, bresult);
        if ( bresult !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of unsigned longint in task: %0d mismatchs %0d", bresult, s_sum(ar, ar_xz));
            $finish;
          end
        // checking longint sum from module
        if ( mcaresult !== s_sum(ar, ar_xz) || mabresult !== s_sum(ar, ar_xz))
          begin
            $display ("FAILED - incorrect addition of signed longtint from module");
            $finish;
          end
      end
    // trying signed mults
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = { $random % 32'd32768, $random % 32'd16384 };
        ar_xz = { $random % 32'd16384, $random % 32'd32768 };
        #1;
        bresult = bu * bu_xz; // truncated mult
        #1;
        if ( bresult !== sh_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect multiplication of signed longints: %0d mismatchs %0d", bresult, sh_mul(ar, ar_xz));
            $finish;
          end
        #1;
        pv1 = $random;
        pv2 = $random;
        #1;
        bresult = pv1 * pv2;              // longint = int x int
        #1;
        if ( bresult !== s_mul(pv1, pv2) )
          begin
            $display ("FAILED - incorrect multiplication of signed longints for int inputs");
            $finish;
          end
        // invoking longint mult function (int*int)
        if ( fs_mul (pv1, pv2) !== s_mul(pv1, pv2) )
          begin
            $display ("FAILED - incorrect product of signed ints for a function returning signed longint");
            $finish;
          end
        // invoking longint mult task (int*int)
        ts_mul (pv1, pv2, bresult);
        if ( bresult !== s_mul(pv1, pv2) )
          begin
            $display ("FAILED - incorrect product of signed int in task returning signed longint");
            $finish;
          end
      end
    // trying relational operators
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = { $random, $random };
        ar_xz = { $random, $random };
        #1;
        if ( (bu < bu_xz ) != (ar < ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than' on signed longints");
            $finish;
          end
        if ( (bu <= bu_xz ) != (ar <= ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than or equal' on signed longints");
            $finish;
          end
        if ( (bu > bu_xz ) != (ar > ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than' on signed longints");
            $finish;
          end
        if ( (bu >= bu_xz ) != (ar >= ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than or equal' than on signed longints");
            $finish;
          end
        if ( (bu == bu_xz ) != (ar == ar_xz) )
          begin
            $display ("FAILED - incorrect 'equal to' on signed longints");
            $finish;
          end
        if ( (bu != bu_xz ) != (ar != ar_xz) )
          begin
            $display ("FAILED - incorrect 'not equal to' on signed ints");
            $finish;
          end
      end
    # 1;
    // signed small number to unsigned shorint
    for (i = 0; i < N_REPS; i = i+1)
      begin
        #1;
        slice = $random % 'h7fff_ffff;
        force bu = slice;
        ar = slice;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect signed extend to signed longint");
            $finish;
          end
      end
    release bu;
    // trying concatenations (and replication)
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        pt1 = $random % MAX8;
        pt2 = $random % MAX8;
        #1;
        bresult = { {4{pt1}}, {4{pt2}} };
        #1;
        if ( bresult[63:56] !== pt1 || bresult[55:48] !== pt1 || bresult[47:40] !== pt1 || bresult[39:32] !== pt1 ||
            bresult[31:24] !== pt2 || bresult[23:16] !== pt2 || bresult[15:8] !== pt2 || bresult[7:0] !== pt2)
          begin
            $display ("FAILED - incorrect concatenation and replication of bytes into signed longints");
            $finish;
          end
        #1;
        ps1 = $random % MAX16;
        ps2 = $random % MAX16;
        #1;
        bresult = { {2{ps1}}, {2{ps2}} };
        #1;
        if ( bresult[63:48] !== ps1 || bresult[47:32] !== ps1 || bresult[31:16] !== ps2 || bresult[15:0] !== ps2)
          begin
            $display ("FAILED - incorrect concatenation and replication of shortint into signed long ints");
            $finish;
          end
        #1;
        pv1 = $random;
        pv2 = $random;
        #1;
        bresult = { pv1, pv2 };
        #1;
        if ( bresult[63:32] !== pv1 || bresult[31:0] !== pv2)
          begin
            $display ("FAILED - incorrect concatenation and replication of int into signed longints");
            $finish;
          end
      end
    #1;
    $display("PASSED");
  end

  // this returns X and Z states into bit random positions for a value
  function [LEN-1:0] xz_inject (input signed [LEN-1:0] value);
      integer i, k;
      time temp;
      begin
        temp = {$random, $random};
        for (i=0; i<LEN; i=i+1)
          begin
             if (temp[i] == 1'b1)
               begin
                 k = $random;
                 if (k <= 0)
                    value[i] = 1'bx;  // 'x noise
                 else
                    value[i] = 1'bz;  // 'z noise
               end
          end
          xz_inject = value;
      end
  endfunction

  // this function returns bit positions with either X or Z to 0 for an input value
  function [LEN-1:0] xz_expected (input signed [LEN-1:0] value_xz);
      integer i;
      begin
        for (i=0; i<LEN; i=i+1)
          begin
             if (value_xz[i] === 1'bx || value_xz[i] === 1'bz )
                 value_xz[i] = 1'b0;  // forced to zero
          end
          xz_expected = value_xz;
      end
  endfunction

  // signed 4-value sum
  function signed [LEN-1:0] s_sum (input signed [LEN-1:0] a, b);
      s_sum = a + b;
  endfunction

   // signed longint sum as function
  function longint signed fs_sum (input longint signed a, b);
      fs_sum = a + b;
  endfunction

  // unsigned longint sum as task
  task ts_sum (input longint signed a, b, output longint signed c);
      c = a + b;
  endtask

   // unsigned 4-value truncated mults
  function signed [LEN-1:0] sh_mul (input signed [LEN-1:0] a, b);
      sh_mul = a * b;
  endfunction

  // unsigned 4-value mults
  function signed [LEN-1:0] s_mul (input signed [LEN/2-1:0] a, b);
      s_mul = a * b;
  endfunction

  // unsigned longint mult as function
  function longint signed fs_mul (input int signed a, b);
      fs_mul = a * b;
  endfunction

  // unsigned longint mult as task
  task ts_mul (input int signed a, b, output longint signed c);
      c = a * b;
  endtask



endmodule
