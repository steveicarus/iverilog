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
#ident "$Id: net_event.cc,v 1.10 2000/09/19 03:00:36 steve Exp $"
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
      wlist_ = 0;
}

NetEvent::~NetEvent()
{
      assert(waitref_ == 0);
      if (scope_) scope_->rem_event(this);
      while (probes_) {
	    NetEvProbe*tmp = probes_->enext_;
	    delete probes_;
	    probes_ = tmp;
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

NetEvent* NetEvent::find_similar_event()
{
      if (probes_ == 0)
	    return 0;
#define NCAND 256
      NetEvent*cand[NCAND];
      bool cflg[NCAND];
      unsigned ncand = 0;

      NetEvProbe*cur = probes_;

	/* First locate all the canditate events from the probe
	   objects that are connected to them and to my first
	   probe. All of these candidates have at least this probe in
	   common. */

      for (NetNode*idx = cur->next_node()
		 ; idx && (idx != cur) ;  idx = idx->next_node()) {
	    NetEvProbe*tmp = dynamic_cast<NetEvProbe*>(idx);
	    if (tmp == 0)
		  continue;
	    if (tmp->edge() != cur->edge())
		  continue;

	    cand[ncand++] = tmp->event();
	    assert(ncand <= NCAND);
      }

	/* Now go through the remaining probes, checking that all the
	   candidate events also have a probe connected to this
	   one. By the time I finish this list, I will have eliminated
	   all the candidate events that are not connected to all of
	   my probes. */

      for (cur = cur->enext_ ;  cur && ncand ;  cur = cur->enext_) {
	    for (unsigned idx = 0 ;  idx < ncand ;  idx += 1)
		  cflg[idx] = false;

	      /* For this probe, look for other probes connected to it
		 and find the event connected to it. Mark that event
		 as connected to this probe. */

	    for (NetNode*idx = cur->next_node()
		       ; idx && (idx != cur) ;  idx = idx->next_node()) {
		  NetEvProbe*tmp = dynamic_cast<NetEvProbe*>(idx);
		  if (tmp == 0)
			continue;
		  if (tmp->edge() != cur->edge())
			continue;

		  for (unsigned srch = 0 ;  srch < ncand ;  srch += 1)
			if (cand[srch] == tmp->event()) {
			      cflg[srch] = true;
			      break;
			}
	    }

	      /* Look for all the candidates that did not connect to
		 this probe (cflg is false) and eliminate them. */

	    for (unsigned idx = 0 ;  idx < ncand ;  ) {
		  if (cflg[idx]) {
			idx += 1;
			continue;
		  }

		  for (unsigned tmp = idx ;  (tmp+1) < ncand ;  tmp += 1) {
			cflg[tmp] = cflg[tmp+1];
			cand[tmp] = cand[tmp+1];
		  }
		  ncand -= 1;
	    }
      }

	/* Now, skip any of the remaining candidates for any that have
	   a different number of probes. These would be events that
	   have probes that I'm not connected to. */

      for (unsigned idx = 0 ;  idx < ncand ;  idx += 1) {
	    if (cand[idx]->nprobe() == nprobe())
		  return cand[idx];
      }

      return 0;
}

void NetEvent::replace_event(NetEvent*that)
{
      while (wlist_) {
	    wlist_->obj->replace_event(this, that);
      }
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

NetEvent* NetEvProbe::event()
{
      return event_;
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

		  struct NetEvent::wcell_*tmp = tgt->wlist_;
		  if (tmp->obj == this) {
			tgt->wlist_ = tmp->next;
			delete tmp;
		  } else {
			assert(tmp->next);
			while (tmp->next->obj != this) {
			      tmp = tmp->next;
			      assert(tmp->next);
			}
			tmp->next = tmp->next->next;
			delete tmp;
		  }
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

      struct NetEvent::wcell_*tmp = new NetEvent::wcell_;
      tmp->obj = this;
      tmp->next = tgt->wlist_;
      tgt->wlist_ = tmp;
}

void NetEvWait::replace_event(NetEvent*src, NetEvent*repl)
{
      unsigned idx;
      for (idx = 0 ;  idx < nevents_ ;  idx += 1) {
	    if (events_[idx] == src)
		  break;
      }

      assert(idx < nevents_);

	/* First, remove me from the list held by the src NetEvent. */
      assert(src->waitref_ > 0);
      src->waitref_ -= 1;
      struct NetEvent::wcell_*tmp = src->wlist_;
      if (tmp->obj == this) {
	    src->wlist_ = tmp->next;
	    delete tmp;
      } else {
	    assert(tmp->next);
	    while (tmp->next->obj != this) {
		  tmp = tmp->next;
		  assert(tmp->next);
	    }
	    tmp->next = tmp->next->next;
	    delete tmp;
      }

      events_[idx] = repl;

	// Remember to tell the replacement NetEvent that there is
	// someone pointing to it.
      repl->waitref_ += 1;

      tmp = new NetEvent::wcell_;
      tmp->obj = this;
      tmp->next = repl->wlist_;
      repl->wlist_ = tmp;

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
 * Revision 1.10  2000/09/19 03:00:36  steve
 *  Typo stepping ot next probe in delete.
 *
 * Revision 1.9  2000/07/29 03:55:38  steve
 *  fix problem coalescing events w/ probes.
 *
 * Revision 1.8  2000/05/31 02:26:49  steve
 *  Globally merge redundant event objects.
 *
 * Revision 1.7  2000/05/27 19:33:23  steve
 *  Merge similar probes within a module.
 *
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

