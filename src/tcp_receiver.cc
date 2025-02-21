#include "tcp_receiver.hh"
#include "byte_stream.hh"
#include "debug.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <optional>
#include <iostream>

using namespace std;
using std::cout, std::endl;

void TCPReceiver::receive( TCPSenderMessage message )
{
  cout<<message.SYN<<" "<<message.FIN<<" "<<message.payload<<" "<<message.seqno.raw()<<endl;
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
  if(message.FIN){
    FIN_ = true;
  }
  message.seqno.unwrap(zero_point_, check_point_);
  check_point_ += message.sequence_length();
  reassembler.insert(first_index, message.payload, FIN_);
}

TCPReceiverMessage TCPReceiver::send() const
{
  std::optional<Wrap32> ack = nullopt;
  if(SYN_){
    ack = Wrap32::wrap(this->reassembler_.current_index() + SYN_ + FIN_, zero_point_);
                                                              //here is
  }
  return {ack ,static_cast<uint16_t>(this->writer().available_capacity()), 0};
}
