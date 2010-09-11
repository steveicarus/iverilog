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

# include  "config.h"
# include  "compiler.h"
# include  "netlist.h"
# include  <set>

using namespace std;

/*
 * NOTE: The name_ is perm-allocated by the caller.
 */
NetEvent::NetEvent(perm_string n)
: name_(n)
{
      scope_ = 0;
      snext_ = 0;
      probes_ = 0;
      trig_ = 0;
      waitref_ = 0;
      exprref_ = 0;
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
	/* name_ is lex_strings. */
}

perm_string NetEvent::name() const
{
      return name_;
}

NetScope* NetEvent::scope()
{
      assert(scope_);
      return scope_;
}

const NetScope* NetEvent::scope() const
{
      assert(scope_);
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

static bool probes_are_similar(NetEvProbe*a, NetEvProbe*b)
{
      if ( a->edge() != b->edge() )
            return false;

      set<const Nexus*> aset;
      for (unsigned idx = 0 ; idx < a->pin_count() ; idx += 1)
	    aset .insert( a->pin(idx).nexus() );

      set<const Nexus*> bset;
      for (unsigned idx = 0 ; idx < b->pin_count() ; idx += 1)
	    bset .insert( b->pin(idx).nexus() );

      return aset == bset;
}

static bool events_are_similar(NetEvent*a, NetEvent*b)
{
      if (a->nprobe() != b->nprobe())
	    return false;

      vector<bool> amatched (a->nprobe());
      vector<bool> bmatched (b->nprobe());
      for (unsigned adx = 0 ; adx < a->nprobe() ; adx += 1) {
	    NetEvProbe*ap = a->probe(adx);
	    for (unsigned bdx = 0 ; bdx < b->nprobe() ; bdx += 1) {
		  if (bmatched[bdx]) continue;

		  NetEvProbe*bp = b->probe(bdx);
		  if (! probes_are_similar(ap, bp)) continue;

		  amatched[adx] = true;
		  bmatched[bdx] = true;
		  break;
	    }
      }

      for (unsigned idx = 0 ; idx < amatched.size() ; idx += 1)
	    if (amatched[idx] == false) return false;

      for (unsigned idx = 0 ; idx < bmatched.size() ; idx += 1)
	    if (bmatched[idx] == false) return false;

      return true;
}

/*
 * A "similar" event is one that has an identical non-nil set of
 * probes.
 */
void NetEvent::find_similar_event(list<NetEvent*>&event_list)
{
      if (probes_ == 0)
	    return;

      set<NetEvent*> event_candidates;
      list<NetEvProbe*>first_probes;
      probes_->find_similar_probes(first_probes);

      for (list<NetEvProbe*>::iterator idx = first_probes.begin()
		 ; idx != first_probes.end() ; idx ++) {
	    event_candidates .insert( (*idx)->event() );
      }

      for (set<NetEvent*>::iterator idx = event_candidates.begin()
		 ; idx != event_candidates.end() ; idx ++) {
	    if (this == *idx)
		  continue;
	    if (scope()->is_auto() && (scope() != (*idx)->scope()))
		  continue;
	    if (events_are_similar(this, *idx))
		  event_list.push_back(*idx);
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

	    for (unsigned idx = 0 ;  idx < cur->pin_count() ;  idx += 1)
		  tmp->add(cur->pin(idx).nexus());

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
	    for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1)
		  if (pin(idx).nexus() != tmp->pin(idx).nexus()) {
			ok_flag = false;
			break;
		  }

	    if (ok_flag == true)
		  plist .push_back(tmp);
      }
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
