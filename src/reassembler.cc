#include "reassembler.hh"
#include "debug.hh"
#include <compare>
#include <cstdint>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <vector>

using std::move;
using std::cout;
using std::endl;


using namespace std;

Reassembler::Reassembler( ByteStream&& output ) : output_( std::move( output ) ), buffer_(), current_index_( 0 ), buffer_size_( available_capacity() ), close_index_(0x3f3f3f3f) {}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(is_last_substring){
    close_index_ = first_index + data.length();
    if(close_index_ == 0 || close_index_ == current_index_){
      output_.writer().close();
      return;
    }
  }
  
  if(first_index < current_index_){
    if(current_index_ - first_index < data.length()){
      data = data.substr(current_index_ - first_index, data.length() - (current_index_ - first_index));
      first_index = current_index_;
    }else{
      return;
    }
  }
  auto iter = buffer_.lower_bound(first_index);
  uint64_t offset = 0, length = data.length();
  
  if(iter != buffer_.begin()){
    iter--;
    if(iter->first + iter->second.length() > first_index){
      offset = iter->first + iter->second.length() - first_index;
      length -= offset;
      first_index += offset;
    }
    iter++;
  }

  if(length > 0){
    std::vector<uint64_t> remove_list;
    while(iter != buffer_.end() && iter->first < first_index + length){
      if(iter->first + iter->second.length() < first_index + length){
                                                            //here is the last bug, former is data.length()
        remove_list.push_back(iter->first);
      }else{
        length -= first_index + length - iter->first;
      } 
      iter++; 
    }
    for(auto key : remove_list){
      buffer_.erase(key);
    }
  }

  uint64_t last_index = current_index_ + available_capacity();
  length = min(last_index - first_index, length);

  if( first_index < last_index && length > 0 && offset < data.length()){
    buffer_.insert({first_index, data.substr(offset, length)});
  }

  bool close_flag = false;
  while(1){
    if(!(!buffer_.empty() && buffer_.begin()->first == current_index_)){
      break;
    }
    std::string cur;
    cur = move( buffer_.begin()->second );
    buffer_.erase(buffer_.begin());
    uint64_t len = cur.length();
    if(length + current_index_ >= close_index_){
      close_flag = true;
    }
    output_.writer().push(cur);
    current_index_ += len;
  }
  if(close_flag){
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t size = 0;
  for(const auto& [i,value] : buffer_){
    size += value.length();
  }
  return size;
}
