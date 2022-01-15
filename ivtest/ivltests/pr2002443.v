`define MACRO(_param_,_def_) \
`ifdef _def_ \
module _param_ (); \
endmodule \
`endif

`MACRO(FOFO, CFG_FOFO)
