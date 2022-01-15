module top;
  // An undefined sequence value is an error.
  enum {udef[1'bx]} udef1;
  enum {udef1[1'bx:1]} udef2;
  enum {udef2[1:1'bx]} udef3;
  enum {udefb[1'bx:1'bx]} udef4;
  enum {uval[1'bx] = 1} uval1;
  enum {uval1[1'bx:1] = 1} uval2;
  enum {uval2[1:1'bx] = 1} uval3;
  enum {uvalb[1'bx:1'bx] = 1} uval4;

  // A zero sequence value is an error.
  enum {zdef[0]} zdef1;
  enum {zval[0] = 1} zval1;

  // A negative sequence value is an error.
  enum {ndef[-1]} ndef1;
  enum {ndef1[-1:0]} ndef2;
  enum {ndef2[0:-1]} ndef3;
  enum {ndefb[-1:-1]} ndef4;
  enum {nval[-1] = 1} nval1;
  enum {nval1[-1:0] = 1} nval2;
  enum {nval2[0:-1] = 1} nval3;
  enum {nvalb[-1:-1] = 1} nval4;

  initial $display("FAILED");
endmodule
