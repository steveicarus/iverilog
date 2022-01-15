// Eleven basic tests in here:
// 1. int must be initialised before any initial or always block
// 2. assignments to (signed) int with random numbers
// 3. assignments to (signed) int with random values including X and Z
// 4. converting unsigned integers to signed int
// 5. converting signed integers to signed int
// 6. converting integers including X and Z states to signed int
// 7. trying signed sums (procedural, function, task and module)
// 8. trying signed mults (procedural, function and task)
// 9. trying relational operators
// 10. smaller signed numbers to signed int (signed extension)
// 11. trying some concatenations from bytes, shortints to ints

module ms_add (input int signed a, b, output int signed sc, ss);
   assign sc = a + b;
   always @(a, b) ss = a + b;
endmodule

module main;
  parameter N_REPS = 500;               // repetition with random numbers
  parameter XZ_REPS = 500;              // repetition with 'x 'z values
  parameter UMAX = 'h7fff_ffff;
  parameter MAX8 = 'h7f;
  parameter MAX16 = 'h7fff;
  parameter LEN = 32;
  // variables used as golden references
  reg signed [LEN-1:0] ar;              // holds numbers
  reg signed [LEN-1:0] ar_xz;           // holds 'x and/or 'z in random positions
  reg signed [LEN-1:0] ar_expected;
  integer unsigned       ui;
  integer signed         si;
  reg signed [LEN/2-1:0]       slice;

  // type assumed tested before
  byte     signed pt1, pt2;
  shortint signed ps1, ps2;

  // types to be tested
  int signed bu;                       // holds numbers
  int signed bu_xz;                    // 'x and 'z are attempted on this
  int signed bresult;                  // hold results from sums and mults
  int signed mcaresult;                // wired to a module instance
  int signed mabresult;                // also wired to a module instance


  integer i;

  // continuous assigments
  // type LHS      type RHS
  // ---------     ---------
  // int           4-value logic
  assign bu = ar;
  assign bu_xz = ar_xz;

  // module instantiation
  ms_add duv (.a(bu), .b(bu_xz), .sc(mcaresult), .ss(mabresult) );

  // all test
  initial begin
    // time 0 checkings (Section 6.4 of IEEE 1850 LRM)
    if (bu !== 32'b0 || bu_xz !== 32'b0 || bresult !== 32'b0 || mcaresult !== 32'b0 || mabresult !== 32'b0)
      begin
        $display ("FAILED - time zero initialisation incorrect: %b %b", bu, bu_xz);
        $finish;
      end
    // driving int type with signed random numbers from a variable
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = $random;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect assigment to int: %b", bu);
            $finish;
        end
      end
    # 1;
    // attempting to drive variables having 'x 'z values into type signed int
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        ar = $random;
        ar_xz = xz_inject (ar);
        ar_expected = xz_expected (ar_xz);
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect assigment to int (when 'x 'z): %b", bu);
            $finish;
        end
      end
    // converting unsigned integers to signed int
    // maintaining bit representation is expected
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ui = {$random};
        #1;
        force bu = ui;
        #1;
        if (bu !== ui)
          begin
            $display ("FAILED - incorrect truncation from unsigned integer to int: %b", bu);
            $finish;
        end
      end
    release bu;
    // converting signed integers to signed ints
    //  bit representation should be maintained
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        si = $random;
        #1;
        force bu = si;
        #1;
        if (bu !== si)
          begin
            $display ("FAILED - incorrect assignment from signed integer to int: %b mismatchs %b", bu, si);
            $finish;
        end
      end
    release bu;
    // converting integers having 'x 'z values into type signed int
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    // coercion to zero expected
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        si = $random;
        ar_xz = xz_inject (si);
        si = ar_xz;
        ar_expected = xz_expected (ar_xz);
        #1;
        force bu_xz = si;
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect conversion from integer (with 'x 'z) to int: %b mismatchs %b", bu_xz, ar_expected);
            $finish;
        end
      end
    release bu_xz;
    // trying signed sums
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = $random;
        ar_xz = $random;
        #1;
        bresult = bu + bu_xz;
        #1;
        if ( bresult !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed ints: %0d mismatchs %0d", bresult, s_sum(ar, ar_xz));
            $finish;
          end
        // invoking shortint sum function
        if ( fs_sum (bu, bu_xz) !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed int in function");
            $finish;
          end
        // invoking byte sum task
        ts_sum (bu, bu_xz, bresult);
        if ( bresult !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed int in task: %0d mismatchs %0d", bresult, s_sum(ar, ar_xz));
            $finish;
          end
        // checking byte sum from module
        if ( mcaresult !== s_sum(ar, ar_xz) || mabresult !== s_sum(ar, ar_xz))
          begin
            $display ("FAILED - incorrect addition of signed int from module");
            $finish;
          end
      end
    // trying signed mults
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = ($random % UMAX) << (LEN/2);
        ar_xz = ($random % UMAX) << (LEN/2 - 1);
        #1;
        bresult = bu * bu_xz;              // truncated multiplication
        #1;
        if ( bresult !== sh_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect multiplication of signed ints (truncated)");
            $finish;
          end
        #1;
        ps1 = $random % 'h7fff;
        ps2 = $random % 'h7fff;
        #1;
        bresult = ps1 * ps2;              // int = shorint x shortint
        #1;
        if ( bresult !== s_mul(ps1, ps2) )
          begin
            $display ("FAILED - incorrect multiplication of signed shortints");
            $finish;
          end
        // invoking int mult function (shortint*shortint)
        if ( fs_mul (ps1, ps2) !== s_mul(ps1, ps2) )
          begin
            $display ("FAILED - incorrect product of signed shortint for a function returning signed int");
            $finish;
          end
        // invoking int mult task (shortint*shortint)
        ts_mul (ps1, ps2, bresult);
        if ( bresult !== s_mul(ps1, ps2) )
          begin
            $display ("FAILED - incorrect product of signed shortint in task returning signed int");
            $finish;
          end
      end
    // trying relational operators
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = $random;
        ar_xz = $random;
        #1;
        if ( (bu < bu_xz ) !== (ar < ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than' on signed ints");
            $finish;
          end
        if ( (bu <= bu_xz ) !== (ar <= ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than or equal' on signed ints");
            $finish;
          end
        if ( (bu > bu_xz ) !== (ar > ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than' on signed ints");
            $finish;
          end
        if ( (bu >= bu_xz ) !== (ar >= ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than or equal' than on signed ints");
            $finish;
          end
        if ( (bu == bu_xz ) !== (ar == ar_xz) )
          begin
            $display ("FAILED - incorrect 'equal to' on signed ints");
            $finish;
          end
        if ( (bu != bu_xz ) !== (ar != ar_xz) )
          begin
            $display ("FAILED - incorrect 'not equal to' on signed ints");
            $finish;
          end
      end
    # 1;
    // signed small number to signed int
    for (i = 0; i < (1<<LEN/2); i = i+1)
      begin
        #1;
        pt1 = $random % MAX8;
        pt2 = $random % MAX8;
        #1;
        bresult = { {2{pt1}}, {2{pt2}} };
        #1;
        if ( bresult[31:24] !== pt1 || bresult[23:16] !== pt1 || bresult[15:8] !== pt2 || bresult[7:0] !== pt2)
          begin
            $display ("FAILED - incorrect concatenation and replication of bytes into signed ints");
            $finish;
          end
        #1;
        ps1 = $random % MAX16;
        ps2 = $random % MAX16;
        #1;
        bresult = { ps1, ps2 };
        #1;
        if ( bresult[31:16] !== ps1 || bresult[15:0] !== ps2)
          begin
            $display ("FAILED - incorrect concatenation of shortint into signed ints");
            $finish;
          end
      end
    release bu;
    // trying concatenations and part select
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        pt1 = $random % MAX8;
        pt2 = $random % MAX8;
        #1;
        bresult = {pt1, pt2};
        #1;
        if ( bresult[16:8] !== pt1 || bresult[7:0] !== pt2)
          begin
            $display ("FAILED - incorrect part select in signed ints");
            $finish;
          end
      end
    #1;
    $display("PASSED");
  end

  // this returns X and Z states into bit random positions for a value
  function [LEN-1:0] xz_inject (input signed [LEN-1:0] value);
      integer i, temp;
      begin
        temp = $random;
        for (i=0; i<LEN; i=i+1)
          begin
             if (temp[i] == 1'b1)
               begin
                 temp = $random;
                 if (temp <= 0)
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

  // signed int sum as function
  function int signed fs_sum (input int signed a, b);
      fs_sum = a + b;
  endfunction

  // signed int sum as task
  task ts_sum (input int signed a, b, output int signed c);
      c = a + b;
  endtask

   // signed 4-value truncated mults
  function signed [LEN-1:0] sh_mul (input signed [LEN-1:0] a, b);
      sh_mul = a * b;
  endfunction

   // signed 4-value mults
  function signed [LEN-1:0] s_mul (input signed [LEN/2-1:0] a, b);
      s_mul = a * b;
  endfunction

  // signed int mult as function
  function int signed fs_mul (input shortint signed a, b);
      fs_mul = a * b;
  endfunction

  // signed int mult as task
  task ts_mul (input shortint signed a, b, output int signed c);
      c = a * b;
  endtask

endmodule
