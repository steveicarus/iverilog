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
#ident "$Id: net_event.cc,v 1.1 2000/04/04 03:20:15 steve Exp $"
#endif

# include  "netlist.h"

NetEvent::NetEvent(const string&n)
: name_(n)
{
      scope_ = 0;
      snext_ = 0;
}

NetEvent::~NetEvent()
{
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

NetEvTrig::NetEvTrig(NetEvent*ev)
: event_(ev)
{
}

NetEvTrig::~NetEvTrig()
{
}

const NetEvent* NetEvTrig::event() const
{
      return event_;
}

NetEvWait::NetEvWait(NetEvent*ev, NetProc*pr)
: event_(ev), statement_(pr)
{
}

NetEvWait::~NetEvWait()
{
      delete statement_;
}

const NetEvent* NetEvWait::event() const
{
      return event_;
}

/*
 * $Log: net_event.cc,v $
 * Revision 1.1  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 */

