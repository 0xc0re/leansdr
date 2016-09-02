#ifndef LEANSDR_GENERIC_H
#define LEANSDR_GENERIC_H

#include <sys/types.h>
#include <unistd.h>

namespace leansdr {

//////////////////////////////////////////////////////////////////////
// Simple blocks
//////////////////////////////////////////////////////////////////////

// [file_reader] reads raw data from a file descriptor into a [pipebuf].
// If the file descriptor is seekable, data can be looped.

template<typename T>
struct file_reader : runnable {
  file_reader(scheduler *sch, int _fdin, pipebuf<T> &_out)
    : runnable(sch, _out.name),
      loop(false),
      fdin(_fdin), out(_out),
      pos(0) {
  }
  void run() {
    size_t size = out.writable() * sizeof(T);
    if ( ! size ) return;
  again:
    ssize_t nr = read(fdin, out.wr(), size);
    if ( nr < 0 ) fatal("read");
    if ( !nr && !loop ) return;
    if ( ! nr ) {
      if ( sch->debug ) fprintf(stderr, "%s looping\n", name);
      off_t res = lseek(fdin, 0, SEEK_SET);
      if ( res == (off_t)-1 ) fatal("lseek");
      goto again;
    }
    if ( nr % sizeof(T) ) fatal("partial read");
    out.written(nr / sizeof(T));
  }
  bool loop;
private:
  int fdin;
  pipewriter<T> out;
  off_t pos;
};

// [file_writer] writes raw data from a [pipebuf] to a file descriptor.

template<typename T>
struct file_writer : runnable {
  file_writer(scheduler *sch, pipebuf<T> &_in, int _fdout) :
    runnable(sch, _in.name),
    in(_in), fdout(_fdout) {
  }
  void run() {
    int size = in.readable() * sizeof(T);
    if ( ! size ) return;
    int nw = write(fdout, in.rd(), size);
    if ( ! nw ) fatal("pipe");
    if ( nw < 0 ) fatal("write");
    if ( nw % sizeof(T) ) fatal("partial write");
    in.read(nw/sizeof(T));
  }
private:
  pipereader<T> in;
  int fdout;
};

// [file_printer] writes data from a [pipebuf] to a file descriptor,
// with printf-style formatting and optional scaling.

template<typename T>
struct file_printer : runnable {
  file_printer(scheduler *sch, const char *_format,
	      pipebuf<T> &_in, int _fdout) :
    runnable(sch, _in.name),
    scale(1), in(_in), format(_format), fdout(_fdout) {
  }
  void run() {
    int n = in.readable();
    T *pin=in.rd(), *pend=pin+n;
    for ( ; pin<pend; ++pin ) {
      char buf[256];
      int len = snprintf(buf, sizeof(buf), format, (*pin)*scale);
      if ( len < 0 ) fatal("obsolete glibc");
      int nw = write(fdout, buf, len);
      if ( nw != len ) fatal("partial write");
    }
    in.read(n);
  }
  T scale;
private:
  pipereader<T> in;
  const char *format;
  int fdout;
};

// [itemcounter] writes the number of input items to the output [pipebuf].
// [Tout] must be a numeric type.

template<typename Tin, typename Tout>
struct itemcounter : runnable {
  itemcounter(scheduler *sch, pipebuf<Tin> &_in, pipebuf<Tout> &_out)
    : runnable(sch, "itemcounter"),
      in(_in), out(_out) {
  }
  void run() {
    if ( out.writable() < 1 ) return;
    unsigned long count = in.readable();
    if ( ! count ) return;
    *out.wr() = count;
    in.read(count);
    out.written(1);
  }
private:
  pipereader<Tin> in;
  pipewriter<Tout> out;
};

}  // namespace

#endif  // LEANSDR_GENERIC_H
