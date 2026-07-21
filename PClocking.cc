/*
 * Minimal SystemVerilog clocking-block parse representation.
 */

# include "config.h"

# include  "PClocking.h"

PClocking::PClocking(perm_string n)
: name_(n)
{
}

PClocking::~PClocking()
{
}

void PClocking::set_events(const std::vector<PEEvent*>&evs)
{
      events_ = evs;
}

PNamedItem::SymbolType PClocking::symbol_type() const
{
      return CLOCKING;
}
