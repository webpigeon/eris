#ifndef ERIS_POLL_GLIB_H
#define ERIS_POLL_GLIB_H

#include "PollGlibVersion.h"
#include "PollGlibFD.h"
#include "PollGlibSource.h"

#include <map>
#include "Poll.h"
#include "Types.h"
#include "Timeout.h"

namespace Eris {

class PollGlib : public Eris::Poll, public Eris::PollData, public PollGlibSource
{
public:
#ifdef ERIS_POLL_GLIB_2_0
  PollGlib(GMainContext *con = 0) : PollGlibSource(con), _wait_time(0) {}
#else
  PollGlib() : _wait_time(0) {}
#endif
  virtual ~PollGlib()
  {
    for(StreamMap::iterator I = _streams.begin(); I != _streams.end(); ++I)
      delete I->second;
  }

  virtual void addStream(const basic_socket_stream* str, Check check)
  {
    if(!(check & MASK))
      throw Eris::InvalidOperation("Null check in PollGlib");

    gushort events = getEvents(check);

#ifdef ERIS_POLL_GLIB_2_0
    PollGlibFD* fd = new PollGlibFD(source(), str, events);
#else
    PollGlibFD* fd = new PollGlibFD(str, events);
#endif

    if(!_streams.insert(StreamMap::value_type(str, fd)).second) {
      delete fd;
      throw Eris::InvalidOperation("Duplicate streams in PollGlib");
    }
  }

  virtual void changeStream(const basic_socket_stream* str, Check check)
  {
    if(!(check & MASK))
      throw Eris::InvalidOperation("Null check in PollGlib");

    StreamMap::iterator I = _streams.find(str);

    if(I == _streams.end())
      throw Eris::InvalidOperation("Can't find stream in PollGlib");

    I->second->setEvents(getEvents(check));
  }

  virtual void removeStream(const basic_socket_stream* str)
  {
    StreamMap::iterator I = _streams.find(str);

    if(I == _streams.end())
      throw Eris::InvalidOperation("Can't find stream in PollGlib");

    PollGlibFD *data = I->second;

    _streams.erase(I);

    delete data;
  }

  virtual bool isReady(const basic_socket_stream* str)
  {
    StreamMap::iterator I = _streams.find(str);

    return I != _streams.end() && I->second->check();
  }

protected:
  virtual bool prepare(int& timeout)
  {
    timeout = _wait_time;
    return timeout != -1;
  }
  virtual bool check()
  {
    for(StreamMap::iterator I = _streams.begin(); I != _streams.end(); ++I)
      if(I->second->check())
        return TRUE;

    return FALSE;
  }
  virtual bool dispatch()
  {
    if(check())
      emit(*this);

    unsigned long wait = Eris::Timeout::pollAll();

    if(new_timeout_) {
      _wait_time = 0;
       new_timeout_ = false;
    }
    else if(wait <= G_MAXINT)
      _wait_time = wait;
    else
      _wait_time = -1;

    return TRUE;
  }

 private:
  static gushort getEvents(Eris::Poll::Check check)
  {
    assert(check & Eris::Poll::MASK);

    gushort events = 0;

    if(check & Eris::Poll::READ)
      events |= G_IO_IN;
    if(check & Eris::Poll::WRITE)
      events |= G_IO_OUT;

    assert(events);

    return events;
  }

  typedef std::map<const basic_socket_stream*,PollGlibFD*> StreamMap;
  StreamMap _streams;

  gint _wait_time;
};

} // namespace Eris

#endif // ERIS_POLL_GLIB_H