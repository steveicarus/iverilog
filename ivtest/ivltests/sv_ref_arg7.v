// Ref arguments with recursion, concurrent calls, the automatic
// variables of the caller, virtual methods and other data types.

typedef enum {A, B, C} en_t;
typedef struct packed { logic [3:0] hi; logic [3:0] lo; } ps_t;

class base;
  virtual task setv(ref int x, input int n);
    x = n;
  endtask
endclass

class derived extends base;
  virtual task setv(ref int x, input int n);
    x = n + 100;
  endtask
endclass

module top;
  int g1, g2, cnt;
  en_t e;
  ps_t p;
  bit [7:0] b8;
  byte sb;
  longint li;
  integer ii;
  time tm;
  shortreal sr;

  task automatic rec(ref int c, input int n);
    if (n == 0) return;
    c = c + 1;
    rec(c, n - 1);
  endtask

  task automatic slow(ref int x, input int d, input int v);
    #d;
    x = v;
  endtask

  // The variable passed by reference is automatic, and is written
  // while the caller waits.
  task automatic outer(output int res);
    int loc;
    loc = 0;
    fork
      slow(loc, 5, 11);
      begin
        #1;
        if (loc !== 0) $display("FAILED: loc=%0d at 1", loc);
        #6;
        if (loc !== 11) $display("FAILED: loc=%0d at 7", loc);
      end
    join
    res = loc;
  endtask

  task automatic set_en(ref en_t x); x = C; endtask
  task automatic set_ps(ref ps_t x); x.lo = 4'h9; endtask
  task automatic inc_b8(ref bit [7:0] x); x = x + 1; endtask
  task automatic set_sb(ref byte x); x = -3; endtask
  task automatic set_li(ref longint x); x = 64'h1_0000_0000; endtask
  task automatic set_ii(ref integer x); x = 77; endtask
  task automatic set_tm(ref time x); x = 99; endtask
  task automatic set_sr(ref shortreal x); x = 2.5; endtask

  initial begin
    base bs;
    derived dv;
    int r, a1, a2;

    cnt = 0;
    rec(cnt, 5);
    if (cnt !== 5) begin
      $display("FAILED: cnt=%0d (expect 5)", cnt);
      $finish;
    end

    // Calls that overlap in time use their own arguments.
    g1 = 0;
    g2 = 0;
    fork
      slow(g1, 3, 31);
      slow(g2, 6, 62);
    join
    if (g1 !== 31 || g2 !== 62) begin
      $display("FAILED: g1=%0d g2=%0d (expect 31, 62)", g1, g2);
      $finish;
    end

    outer(r);
    if (r !== 11) begin
      $display("FAILED: r=%0d (expect 11)", r);
      $finish;
    end

    e = A;
    p = 8'h00;
    b8 = 8'hff;
    set_en(e);
    set_ps(p);
    inc_b8(b8);
    set_sb(sb);
    set_li(li);
    set_ii(ii);
    set_tm(tm);
    set_sr(sr);
    if (e !== C || p !== 8'h09 || b8 !== 0 || sb !== -3 ||
        li !== 64'h1_0000_0000 || ii !== 77 || tm !== 99 || sr != 2.5) begin
      $display("FAILED: e=%0d p=%h b8=%0d sb=%0d li=%h ii=%0d tm=%0d sr=%f",
               e, p, b8, sb, li, ii, tm, sr);
      $finish;
    end

    // The call of a virtual method passes the argument to the override.
    dv = new;
    bs = dv;
    bs.setv(a1, 5);
    bs = new;
    bs.setv(a2, 5);
    if (a1 !== 105 || a2 !== 5) begin
      $display("FAILED: a1=%0d a2=%0d (expect 105, 5)", a1, a2);
      $finish;
    end

    $display("PASSED");
  end
endmodule
