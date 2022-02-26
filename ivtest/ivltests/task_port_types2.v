// Check that it is possible to use SV data types for non-ANSI style task ports

module test;

  typedef logic [7:0] T1;
  typedef struct packed { int i; } T2;
  typedef enum { A } T3;

  task t;
  input reg a;
  input logic b;
  input bit c;
  input logic [3:0] d;
  input bit [3:0][3:0] e;
  input byte f;
  input int g;
  input T1 h;
  input T2 i;
  input T3 j;
  input real k;
  input shortreal l;
  input string m;
  input int n[];
  input int o[$];
  input x;
  input [3:0] y;
  input signed z;
    $display("PASSED");
  endtask

  initial begin
    t('0, '0, '0, '0, '0, '0, '0, '0, '0, A, 0.0, 0.0, "", '{0}, '{0}, '0, '0, '0);
  end

endmodule
