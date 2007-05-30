// NetLatch.h
// Author: Alan M. Feldstein
// This class represents an LPM_LATCH device. In Verilog, storage components of this type can be inferred.
//  The pinout is assigned like so:
//    0  -- Gate
//
//    1  -- Data[0]
//    2  -- Q[0]
//     ...
#ifndef NETLATCH_H
#define NETLATCH_H

#include "netlist.h" // NetNode, NetScope, Link class definitions
#include "StringHeap.h" // perm_string class definition

class NetLatch : public NetNode
{
 public:
  NetLatch( NetScope *, perm_string, unsigned );

  Link &pin_Data( unsigned );
  Link &pin_Q( unsigned );
  Link &pin_Gate();

  const Link &pin_Data( unsigned ) const;
  const Link &pin_Q( unsigned ) const;
  const Link &pin_Gate() const;
}; // end class NetLatch

#endif
