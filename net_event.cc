/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: net_event.cc,v 1.6 2000/04/18 04:50:20 steve Exp $"
#endif

# include  "netlist.h"

NetEvent::NetEvent(const string&n)
: name_(n)
{
      scope_ = 0;
      snext_ = 0;
      probes_ = 0;
      trig_ = 0;
      waitref_ = 0;
}

NetEvent::~NetEvent()
{
      assert(waitref_ == 0);
      if (scope_) scope_->rem_event(this);
      while (probes_) {
	    NetEvProbe*tmp = probes_->enext_;
	    delete probes_;
	    tmp = probes_;
      }
}

string NetEvent::name() const
{
      return name_;
}

string NetEvent::full_name() const
{
      assert(scope_);
      return scope_->name() + "." + name_;
}

unsigned NetEvent::nprobe() const
{
      unsigned cnt = 0;
      NetEvProbe*cur = probes_;
      while (cur) {
	    cnt += 1;
	    cur = cur->enext_;
      }

      return cnt;
}

NetEvProbe* NetEvent::probe(unsigned idx)
{
      NetEvProbe*cur = probes_;
      while (cur && idx) {
	    cur = cur->enext_;
	    idx -= 1;
      }
      return cur;
}

unsigned NetEvent::ntrig() const
{
      unsigned cnt = 0;
      NetEvTrig*cur = trig_;
      while (cur) {
	    cnt += 1;
	    cur = cur->enext_;
      }

      return cnt;
}

unsigned NetEvent::nwait() const
{
      return waitref_;
}

NetEvTrig::NetEvTrig(NetEvent*ev)
: event_(ev)
{
      enext_ = event_->trig_;
      event_->trig_ = this;
}

NetEvTrig::~NetEvTrig()
{
      if (event_->trig_ == this) {
	    event_->trig_ = enext_;

      } else {
	    NetEvTrig*cur = event_->trig_;
	    while (cur->enext_ != this) {
		  assert(cur->enext_);
		  cur = cur->enext_;
	    }

	    cur->enext_ = this->enext_;
      }
}

const NetEvent* NetEvTrig::event() const
{
      return event_;
}

NetEvProbe::NetEvProbe(const string&n, NetEvent*tgt,
		       edge_t t, unsigned p)
: NetNode(n, p), event_(tgt), edge_(t)
{
      for (unsigned idx = 0 ;  idx < p ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name("P", idx);
      }

      enext_ = event_->probes_;
      event_->probes_ = this;
}

NetEvProbe::~NetEvProbe()
{
      if (event_->probes_ == this) {
	    event_->probes_ = enext_;

      } else {
	    NetEvProbe*cur = event_->probes_;
	    while (cur->enext_ != this) {
		  assert(cur->enext_);
		  cur = cur->enext_;
	    }

	    cur->enext_ = this->enext_;
      }
}

NetEvProbe::edge_t NetEvProbe::edge() const
{
      return edge_;
}

const NetEvent* NetEvProbe::event() const
{
      return event_;
}

NetEvWait::NetEvWait(NetProc*pr)
: statement_(pr), nevents_(0), events_(0)
{
}

NetEvWait::~NetEvWait()
{
      if (events_) {
	    for (unsigned idx = 0 ;  idx < nevents_ ;  idx += 1) {
		  NetEvent*tgt = events_[idx];
		  tgt->waitref_ -= 1;
	    }
	    delete[]events_;
      }
      delete statement_;
}

void NetEvWait::add_event(NetEvent*tgt)
{
      assert(tgt);
      if (nevents_ == 0) {
	    events_ = new NetEvent*[1];

      } else {
	    NetEvent**tmp = new NetEvent*[nevents_+1];
	    for (unsigned idx = 0 ;  idx < nevents_ ;  idx += 1) {
		  tmp[idx] = events_[idx];
		  assert(tmp[idx] != tgt);
	    }
	    delete[]events_;
	    events_ = tmp;
      }

      events_[nevents_] = tgt;
      nevents_ += 1;

	// Remember to tell the NetEvent that there is someone
	// pointing to it.
      tgt->waitref_ += 1;
}

unsigned NetEvWait::nevents() const
{
      return nevents_;
}

const NetEvent* NetEvWait::event(unsigned idx) const
{
      assert(idx < nevents_);
      return events_[idx];
}

NetEvent* NetEvWait::event(unsigned idx)
{
      assert(idx < nevents_);
      return events_[idx];
}

NetProc* NetEvWait::statement()
{
      return statement_;
}

/*
 * $Log: net_event.cc,v $
 * Revision 1.6  2000/04/18 04:50:20  steve
 *  Clean up unneeded NetEvent objects.
 *
 * Revision 1.5  2000/04/16 23:32:18  steve
 *  Synthesis of comparator in expressions.
 *
 *  Connect the NetEvent and related classes
 *  together better.
 *
 * Revision 1.4  2000/04/12 20:02:53  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 *
 * Revision 1.3  2000/04/12 04:23:58  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 *
 * Revision 1.2  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.1  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 */

