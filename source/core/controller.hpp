#pragma once
#include <array>
#include <cmath>
#include <string_view>

namespace revontuli {
struct Seat { double temperature=21.5; int heat=2,cool=0,fan=3,recline=0,window=0,shade=0,air=0; bool massage=false; };
struct State {
 std::array<Seat,2> seats{};
 bool ac=true,automatic=true,light=true,playing=false,reading=false,fresh=true;
 int scene=0,brightness=85,volume=45,source=0,output=0,focus=0,roof=0;
};
enum class Result { Applied, Invalid, Disconnected };
// The adapter acknowledges a request before the controller commits it.
// A board adapter must replace this synchronous contract with its device protocol.
struct Device { virtual bool apply(const State&)=0; virtual ~Device()=default; };
struct Simulator final:Device { bool connected=true; bool apply(const State&) override {return connected;} };
class Controller {
 State state_{};
 Device& device_;
public:
 explicit Controller(Device& device):device_(device){}
 const State& state() const {return state_;}
 Result dispatch(int seat,std::string_view key,double value) {
  if(seat<0||seat>1||!std::isfinite(value))return Result::Invalid;
  State next=state_; auto& s=next.seats[seat];
  auto integer=[&](int min,int max){return value>=min&&value<=max&&std::floor(value)==value;};
  if(key=="temperature") {if(value<16||value>30||std::floor(value*2)!=value*2)return Result::Invalid;s.temperature=value;}
  else if(key=="heat"&&integer(0,3)){s.heat=int(value);if(value)s.cool=0;}
  else if(key=="cool"&&integer(0,3)){s.cool=int(value);if(value)s.heat=0;}
  else if(key=="fan"&&integer(0,5)){s.fan=int(value);next.automatic=false;}
  else if(key=="air"&&integer(0,2))s.air=int(value);
  else if(key=="recline"&&integer(0,100))s.recline=int(value);
  else if(key=="window"&&integer(0,100))s.window=int(value);
  else if(key=="shade"&&integer(0,100))s.shade=int(value);
  else if(key=="massage"&&integer(0,1))s.massage=bool(value);
  else if(key=="resetSeat"&&value==1){s.heat=0;s.cool=0;s.massage=false;s.recline=0;}
  else if(key=="ac"&&integer(0,1))next.ac=bool(value);
  else if(key=="automatic"&&integer(0,1))next.automatic=bool(value);
  else if(key=="light"&&integer(0,1))next.light=bool(value);
  else if(key=="scene"&&integer(0,4))next.scene=int(value);
  else if(key=="brightness"&&integer(0,100))next.brightness=int(value);
  else if(key=="volume"&&integer(0,100))next.volume=int(value);
  else if(key=="playing"&&integer(0,1))next.playing=bool(value);
  else if(key=="source"&&integer(0,2))next.source=int(value);
  else if(key=="output"&&integer(0,1))next.output=int(value);
  else if(key=="focus"&&integer(0,2))next.focus=int(value);
  else if(key=="roof"&&integer(0,2))next.roof=int(value);
  else if(key=="reading"&&integer(0,1))next.reading=bool(value);
  else if(key=="fresh"&&integer(0,1))next.fresh=bool(value);
  else return Result::Invalid;
  if(!device_.apply(next))return Result::Disconnected;
  state_=next;return Result::Applied;
 }
};
}
