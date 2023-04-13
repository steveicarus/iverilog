/*
 * Copyright (c) 2000-2022 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "config.h"
# include  "compiler.h"
# include  "netlist.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * NOTE: The name_ is perm-allocated by the caller.
 */
NetEvent::NetEvent(perm_string n)
: name_(n)
{
      local_flag_ = false;
      scope_ = 0;
      snext_ = 0;
      probes_ = 0;
      trig_ = 0;
      waitref_ = 0;
      exprref_ = 0;
      wlist_ = 0;
      nb_trig_ = 0;
}

NetEvent::~NetEvent()
{
      ivl_assert(*this, waitref_ == 0);
      if (scope_) scope_->rem_event(this);
      while (probes_) {
	    NetEvProbe*tmp = probes_->enext_;
	    delete probes_;
	    probes_ = tmp;
      }
	/* name_ is lex_strings. */
}

perm_string NetEvent::name() const
{
      return name_;
}

NetScope* NetEvent::scope()
{
      ivl_assert(*this, scope_);
      return scope_;
}

const NetScope* NetEvent::scope() const
{
      ivl_assert(*this, scope_);
      return scope_;
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

const NetEvProbe* NetEvent::probe(unsigned idx) const
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

unsigned NetEvent::nexpr() const
{
      return exprref_;
}

/*
 * A "similar" event is one that has an identical non-nil set of
 * probes.
 */
void NetEvent::find_similar_event(list<NetEvent*>&event_list)
{
      if (probes_ == 0)
	    return;

      set<NetEvent*> candidate_events;

	/* First, get a list of all the NetEvProbes that are connected
	   to my first probe. Then use that to create a set of
	   candidate events. These candidate events are a superset of
	   the similar events, so I will be culling this list later. */
      list<NetEvProbe*>first_probes;
      probes_->find_similar_probes(first_probes);

      for (list<NetEvProbe*>::iterator idx = first_probes.begin()
		 ; idx != first_probes.end() ; ++ idx ) {

	    candidate_events.insert( (*idx)->event() );
      }

      if (candidate_events.empty())
	    return;

	/* Now scan the remaining probes, in each case checking that
	   the probe event is a candidate event. After each iteration,
	   events that don't have a similar probe will be removed from
	   the candidate_events set. If the candidate_events set
	   becomes empty, then give up. */
      unsigned probe_count = 1;
      for (NetEvProbe*cur = probes_->enext_ ; cur;  cur = cur->enext_) {
	    list<NetEvProbe*>similar_probes;
	    cur->find_similar_probes(similar_probes);

	    set<NetEvent*> candidate_tmp;
	    for (list<NetEvProbe*>::iterator idx = similar_probes.begin()
		       ; idx != similar_probes.end() ; ++ idx ) {

		  NetEvent*tmp = (*idx)->event();
		  if (candidate_events.find(tmp) != candidate_events.end())
			candidate_tmp .insert(tmp);
	    }

	      // None of the candidate events match this probe? Give up!
	    if (candidate_tmp.empty())
		  return;

	    candidate_events = candidate_tmp;
	    probe_count += 1;
      }

        /* Scan the surviving candidate events. We know that they all
	   have probes that match the current event's probes. Check
	   for remaining compatibility details and save the survivors
	   in the event_list that the caller passed. */
      for (set<NetEvent*>::iterator idx = candidate_events.begin()
		 ; idx != candidate_events.end() ; ++ idx ) {

	    NetEvent*tmp = *idx;

	      // This shouldn't be possible?
	    if (tmp == this)
		  continue;

              /* For automatic tasks, the VVP runtime holds state for events
                 in the automatically allocated context. This means we can't
                 merge similar events in different automatic tasks. */
            if (scope()->is_auto() && (tmp->scope() != scope()))
                  continue;

	    unsigned tcnt = 0;
	    for (NetEvProbe*cur = tmp->probes_ ; cur ; cur = cur->enext_)
		  tcnt += 1;

	    if (tcnt == probe_count)
		  event_list .push_back(tmp);
      }

}


void NetEvent::replace_event(NetEvent*that)
{
      while (wlist_) {
	    wlist_->obj->replace_event(this, that);
      }
}

NexusSet* NetEvent::nex_async_()
{
	/* If there are behavioral trigger statements attached to me,
	   then this is not an asynchronous event. */
      if (trig_ != 0)
	    return 0;


      NexusSet*tmp = new NexusSet;
      for (NetEvProbe*cur = probes_ ;  cur != 0 ;  cur = cur->enext_) {
	    if (cur->edge() != NetEvProbe::ANYEDGE) {
		  delete tmp;
		  return 0;
	    }

	    for (unsigned idx = 0 ;  idx < cur->pin_count() ;  idx += 1) {
		  Nexus*nex = cur->pin(idx).nexus();
		  tmp->add(nex, 0, nex->vector_width());
	    }
      }

      return tmp;
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
		  ivl_assert(*this, cur->enext_);
		  cur = cur->enext_;
	    }

	    cur->enext_ = this->enext_;
      }
}

const NetEvent* NetEvTrig::event() const
{
      return event_;
}

NetEvNBTrig::NetEvNBTrig(NetEvent*ev, NetExpr*dly)
: event_(ev), dly_(dly)
{
      enext_ = event_->nb_trig_;
      event_->nb_trig_ = this;
}

NetEvNBTrig::~NetEvNBTrig()
{
      if (event_->nb_trig_ == this) {
	    event_->nb_trig_ = enext_;

      } else {
	    NetEvNBTrig*cur = event_->nb_trig_;
	    while (cur->enext_ != this) {
		  ivl_assert(*this, cur->enext_);
		  cur = cur->enext_;
	    }

	    cur->enext_ = this->enext_;
      }
}

const NetExpr* NetEvNBTrig::delay() const
{
      return dly_;
}

const NetEvent* NetEvNBTrig::event() const
{
      return event_;
}

NetEvProbe::NetEvProbe(NetScope*s, perm_string n, NetEvent*tgt,
		       edge_t t, unsigned p)
: NetNode(s, n, p), event_(tgt), edge_(t)
{
      for (unsigned idx = 0 ;  idx < p ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
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
		  ivl_assert(*this, cur->enext_);
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

/*
 * A similar NetEvProbe is one that is connected to all the same nexa
 * that this probe is connected to, and also is the same edge
 * type. Don't count myself as a similar probe.
 */
void NetEvProbe::find_similar_probes(list<NetEvProbe*>&plist)
{
      Nexus*nex = pin(0).nexus();

      for (Link*lcur = nex->first_nlink(); lcur; lcur = lcur->next_nlink()) {
	    NetPins*obj = lcur->get_obj();
	      // Skip NexusSet objects
	    if (obj == 0)
		  continue;

	    if (obj->pin_count() != pin_count())
		  continue;

	    NetEvProbe*tmp = dynamic_cast<NetEvProbe*>(obj);
	    if (tmp == 0)
		  continue;

	    if (tmp == this)
		  continue;

	    if (edge() != tmp->edge())
		  continue;

	    bool ok_flag = true;
	    for (unsigned idx = 1 ;  ok_flag && idx < pin_count() ;  idx += 1)
		  if (! pin(idx).is_linked(tmp->pin(idx)))
			ok_flag = false;

	    if (ok_flag == true)
		  plist .push_back(tmp);
      }
}

NetEvWait::NetEvWait(NetProc*pr)
: statement_(pr), has_t0_trigger_(false)
{
}

NetEvWait::~NetEvWait()
{
      if (! events_.empty()) {
	    for (unsigned idx = 0 ;  idx < events_.size() ;  idx += 1) {
		  NetEvent*tgt = events_[idx];
		  tgt->waitref_ -= 1;

		  struct NetEvent::wcell_*tmp = tgt->wlist_;
		  if (tmp->obj == this) {
			tgt->wlist_ = tmp->next;
			delete tmp;
		  } else {
			ivl_assert(*this, tmp->next);
			while (tmp->next->obj != this) {
			      tmp = tmp->next;
			      ivl_assert(*this, tmp->next);
			}
			tmp->next = tmp->next->next;
			delete tmp;
		  }
		  delete tgt;
	    }
	    events_.clear();
      }
      delete statement_;
}

void NetEvWait::add_event(NetEvent*tgt)
{
	/* A wait fork is an empty event. */
      if (! tgt) {
	    ivl_assert(*this, events_.empty());
	    events_.push_back(0);
	    return;
      }

      events_.push_back(tgt);

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
      for (idx = 0 ;  idx < events_.size() ;  idx += 1) {
	    if (events_[idx] == src)
		  break;
      }

      ivl_assert(*this, idx < events_.size());

	// First, remove me from the list held by the src NetEvent.
      ivl_assert(*this, src->waitref_ > 0);
      src->waitref_ -= 1;
      struct NetEvent::wcell_*tmp = src->wlist_;
      if (tmp->obj == this) {
	    src->wlist_ = tmp->next;
	    delete tmp;
      } else {
	    ivl_assert(*this, tmp->next);
	    while (tmp->next->obj != this) {
		  tmp = tmp->next;
		  ivl_assert(*this, tmp->next);
	    }
	    tmp->next = tmp->next->next;
	    delete tmp;
      }

	// Replace the src pointer with the repl pointer.
      events_[idx] = repl;

	// Remember to tell the replacement NetEvent that there is
	// someone pointing to it.
      repl->waitref_ += 1;

      tmp = new NetEvent::wcell_;
      tmp->obj = this;
      tmp->next = repl->wlist_;
      repl->wlist_ = tmp;

}

NetProc* NetEvWait::statement()
{
      return statement_;
}

const NetProc* NetEvWait::statement() const
{
      return statement_;
}
