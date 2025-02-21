#include "tcp_receiver.hh"
#include "byte_stream.hh"
#include "debug.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <optional>
#include <iostream>
#include <sys/types.h>

using namespace std;
using std::cout, std::endl;

void TCPReceiver::receive( TCPSenderMessage message )
{
  //cout<<message.SYN<<" "<<message.FIN<<" "<<message.payload<<" "<<message.seqno.raw()<<endl;
  auto& reassembler = this->reassembler_;
  uint64_t first_index = 0;
  if(message.SYN){
    check_point_ = 0;
    SYN_ = true;
    zero_point_ = message.seqno;
  }else{
    first_index --;
    //本身index与absoluteseque有错位
    //但是如果包含SYN则抵消
  }
  first_index += message.seqno.unwrap(zero_point_ , check_point_);

  if(message.FIN){
    FIN_ = 1;
    fin_sequo_ = message.seqno + (message.sequence_length() - 1);
  }

  if(message.RST){
    this->reassembler_.set_error();
  }

  message.seqno.unwrap(zero_point_, check_point_);
  check_point_ += message.sequence_length();
  reassembler.insert(first_index, message.payload, 0);
  if(FIN_ && SYN_){
    if((zero_point_ + (this->reassembler_.current_index() + SYN_) == fin_sequo_)){
      fin_flag_ = 1;
    reassembler.insert(this->reassembler_.current_index(), "", 1);
    }
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  std::optional<Wrap32> ack = nullopt;
  if(SYN_){
    ack = Wrap32::wrap(this->reassembler_.current_index() + SYN_ + fin_flag_, zero_point_);
                                                              //here is
  }
  uint16_t capacity;
  if(this->writer().available_capacity() > 65535){
    capacity = 65535;
  }else{
    capacity = static_cast<uint16_t>(this->writer().available_capacity());
  }
  return {ack , capacity, this->reassembler_.reader().has_error()};
}
