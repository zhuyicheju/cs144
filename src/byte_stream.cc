#include "byte_stream.hh"
//#include <cassert>
#include<iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer(capacity),
  write_mirror(0), read_mirror(0), write_pos(0), read_pos(0), closed(0), pushed_bytes(0),popped_bytes(0)
 {}

bool Writer::is_closed() const
{
  //
  return closed;
}
#define assert(x); \
  if(!(x)){\
    cout<<"assert\n"<<endl;\
  }
void Writer::push( string data )
{
  uint64_t midval = (read_mirror ==  write_mirror ? capacity_ - write_pos : read_pos - write_pos);
  std::copy(data.begin(),
            data.begin() + min(midval, data.length()),
            buffer.begin() + write_pos);
  std::copy(data.begin() + min(midval, data.length()),
            data.begin() + min(midval + (read_mirror == write_mirror ? read_pos : 0), data.length()),
            buffer.begin());

  if(write_mirror == read_mirror){
    if(midval <= data.length()){
      pushed_bytes += capacity_ - write_pos + min(data.length()-midval,read_pos);
      write_pos = min(data.length()-midval,read_pos);
    }else{
      pushed_bytes += data.length();
      write_pos += data.length();
    }
  }else{
    pushed_bytes += min(read_pos-write_pos,data.length());
    write_pos += min(read_pos-write_pos,data.length());
  }

  if(write_mirror == read_mirror){
    if(midval <= data.length())
      write_mirror ^= 1;
  }

  // write_mirror ^= (capacity - write_pos <= min(midval, data.length()) ? 1 : 0);
  // write_pos = (capacity - write_pos <= min(midval, data.length()) ? 
  //             min(midval + (read_mirror == write_mirror ? read_pos : 0), data.length()) - min(midval, data.length())
  //            : write_pos + min(midval, data.length()));
}

void Writer::close()
{
  closed = 1;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - ((read_mirror == write_mirror) ? (write_pos - read_pos) : (capacity_ - (read_pos - write_pos)));
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_bytes;
}

bool Reader::is_finished() const
{
  return closed && (read_mirror == write_mirror && read_pos == write_pos);
}

uint64_t Reader::bytes_popped() const
{
  return popped_bytes;
}

string_view Reader::peek() const
{
  return string_view(&buffer[read_pos], 1);
}

void Reader::pop( uint64_t len )
{
  if(write_mirror == read_mirror){
    popped_bytes += min(len, write_pos - read_pos);
    read_pos += min(len, write_pos - read_pos);
  }else{
    if(len < capacity_ - read_pos){
      popped_bytes += len;
      read_pos += len;
    }else{
      popped_bytes += capacity_ - read_pos + min((len - capacity_ + read_pos), write_pos);
      read_pos = min((len - capacity_ + read_pos), write_pos);
      read_mirror ^= 1;
    }
  }
}

uint64_t Reader::bytes_buffered() const
{
  return (read_mirror == write_mirror) ? (write_pos - read_pos) : (capacity_ - (read_pos - write_pos));
}
