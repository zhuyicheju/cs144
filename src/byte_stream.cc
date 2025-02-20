#include "byte_stream.hh"
#include<iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer(),
  extired(0),closed(0), pushed_bytes(0),popped_bytes(0)
 {}

bool Writer::is_closed() const
{
  return closed;
}
void Writer::push( string data )
{
  if(data.empty() || is_closed() || available_capacity() == 0) return;
  uint64_t size = min(data.length(), available_capacity());
  if(size < data.length()) data.resize(size);
  pushed_bytes += size;
  buffer.emplace(std::move(data));
}

void Writer::close()
{
  closed = 1;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - pushed_bytes + popped_bytes;
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_bytes;
}

bool Reader::is_finished() const
{
  return closed && (pushed_bytes == popped_bytes);
}

uint64_t Reader::bytes_popped() const
{
  return popped_bytes;
}

string_view Reader::peek() const
{
  return buffer.empty() ? string_view {} 
                         : string_view { buffer.front() }.substr( extired );
}

void Reader::pop( uint64_t len )
{
  uint64_t size = min(bytes_buffered(), len);
  popped_bytes += size;
  while(size > 0){
    if(buffer.empty())break;
    uint64_t rest = buffer.front().length() - extired;
    if(rest <= size){
      size -= rest;
      buffer.pop();
      extired = 0;
    }else{
      extired += size;
      size = 0;
    }
  }
}

uint64_t Reader::bytes_buffered() const
{
  return pushed_bytes - popped_bytes;
}
