#ifndef IVL_PClocking_H
#define IVL_PClocking_H
/*
 * Minimal SystemVerilog clocking-block parse representation (UVM Tier A #4).
 */

# include  "PNamedItem.h"
# include  "PScope.h"
# include  "StringHeap.h"
# include  "netlist.h"
# include  <map>
# include  <vector>

class PEEvent;
class PExpr;

/*
 * PClocking holds a parsed clocking block: the clocking event and the
 * list of clocking signals (direction + optional skew expression).
 * Skews beyond #0 are accepted in the grammar but ignored at runtime.
 */
class PClocking : public PNamedItem {

    public:
      struct signal_t {
	    NetNet::PortType direction;
	    PExpr*skew; /* optional; ignored for v1 */
      };

      explicit PClocking(perm_string name);
      ~PClocking() override;

      perm_string name() const { return name_; }

      void set_events(const std::vector<PEEvent*>&evs);
      const std::vector<PEEvent*>& events() const { return events_; }

      std::map<perm_string,signal_t> signals;

      SymbolType symbol_type() const override;

    private:
      perm_string name_;
      std::vector<PEEvent*> events_;

    private: // not implemented
      PClocking(const PClocking&);
      PClocking& operator= (const PClocking&);
};

#endif /* IVL_PClocking_H */
