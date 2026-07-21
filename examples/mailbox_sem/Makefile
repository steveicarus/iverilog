# Build/run mailbox + semaphore smoke test against this tree's install.

ROOT      ?= $(abspath ../..)
PREFIX    ?= $(ROOT)/install
IVERILOG  ?= $(PREFIX)/bin/iverilog
VVP       ?= $(PREFIX)/bin/vvp
VVP_MOD   ?= $(PREFIX)/lib/ivl

OUT := mbx_sem_basic.vvp

.PHONY: all run clean

all: $(OUT)

$(OUT): mbx_sem_basic.sv
	$(IVERILOG) -g2012 -o $@ $<

run: $(OUT)
	$(VVP) -M $(VVP_MOD) $(OUT)

clean:
	rm -f $(OUT)
