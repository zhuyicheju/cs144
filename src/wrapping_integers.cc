#include "wrapping_integers.hh"
#include "debug.hh"
#include <cstdint>
#include <cmath>

using namespace std;

uint64_t inline sub(uint64_t a, uint64_t b){
  return a < b ? (b - a) : (a - b);
}

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
//Convert absolute seqno → seqno.
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
//Convert seqno → absolute seqno
  const uint64_t num = pow(2, 32);
  uint64_t idx = checkpoint / num; 
  uint64_t zero_point_value = zero_point.raw_value_ % num;
  uint64_t this_value = this->raw_value_ % num;
  uint64_t minus = this_value > zero_point_value ? this_value - zero_point_value : num - zero_point_value + this_value;
  uint64_t n = sub(minus + num*(idx+1), checkpoint) < sub(minus + num*idx, checkpoint) ? (minus + num*(idx+1)) : (minus + num*idx);
  n = (sub(minus + num*(idx-1), checkpoint) < sub(minus + num*idx, checkpoint) && sub(minus + num*(idx-1), checkpoint) < sub(minus + num*(idx + 1), checkpoint))? (minus + num*(idx-1)) : n;
  return n;
}
