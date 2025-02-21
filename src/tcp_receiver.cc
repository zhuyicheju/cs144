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
    //由于无符号整数的循环性质，此时firstindex会减成最大整数，但一会会加回来
  }
  //else
  //阅读错误，一开始以为message.fin和message.syn不会同时成立

  if(message.FIN){
    FIN_ = 1;
    fin_sequo_ = message.seqno + (message.sequence_length() - 1);
  }

  if(message.RST){
    reassembler.set_error();
  }

  first_index += message.seqno.unwrap(zero_point_ , check_point_);
  check_point_ += message.sequence_length();
  reassembler.insert(first_index, message.payload, 0);

  if(FIN_ && SYN_ && (zero_point_ + (reassembler.current_index() + SYN_) == fin_sequo_)){
    //这里是个坑
    fin_flag_ = 1;
    reassembler.insert(reassembler.current_index(), "", 1);
  }//原本这个insert是与上面的insert合并在一起的，但是当时字符还没插入，导致currentindex无法及时更新，因此fin_flag可能无效
}

TCPReceiverMessage TCPReceiver::send() const
{
  std::optional<Wrap32> ack = nullopt;
  uint16_t capacity = 0;
  if(SYN_){
    ack = Wrap32::wrap(this->reassembler_.current_index() + SYN_ + fin_flag_, zero_point_);
                                                             //这里是个坑
  }

  if(this->writer().available_capacity() > 65535){
    capacity = 65535;
  }else{
    capacity = static_cast<uint16_t>(this->writer().available_capacity());
  }
  //这里是个坑

  return {ack , capacity, this->reassembler_.reader().has_error()};
}
