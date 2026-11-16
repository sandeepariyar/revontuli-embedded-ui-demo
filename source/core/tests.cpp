#include "controller.hpp"
#include <cassert>
#include <limits>
using namespace revontuli;
int main(){Simulator d;Controller c(d);
 assert(c.dispatch(0,"cool",2)==Result::Applied);assert(c.state().seats[0].heat==0);assert(c.state().seats[1].heat==2);
 c.dispatch(0,"massage",1);c.dispatch(0,"massage",0);assert(c.state().seats[0].cool==2);
 assert(c.dispatch(0,"heat",4)==Result::Invalid);assert(c.dispatch(0,"cool",1.5)==Result::Invalid);
 assert(c.dispatch(2,"heat",1)==Result::Invalid);assert(c.dispatch(0,"temperature",std::numeric_limits<double>::quiet_NaN())==Result::Invalid);
 assert(c.dispatch(0,"temperature",21.3)==Result::Invalid);
 c.dispatch(0,"window",75);c.dispatch(0,"temperature",24);c.dispatch(0,"resetSeat",1);
 assert(c.state().seats[0].window==75&&c.state().seats[0].temperature==24);
 d.connected=false;assert(c.dispatch(0,"heat",3)==Result::Disconnected);assert(c.state().seats[0].heat==0);
 d.connected=true;c.dispatch(0,"fan",2);assert(!c.state().automatic);
 assert(c.dispatch(0,"unknown",1)==Result::Invalid);
}
