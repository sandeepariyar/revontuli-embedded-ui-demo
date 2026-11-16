#include "controller.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
using namespace revontuli;
void stateJson(const State& s){
 std::cout<<"{\"seats\":[";
 for(int i=0;i<2;i++){auto&a=s.seats[i];if(i)std::cout<<',';
 std::cout<<"{\"temperature\":"<<a.temperature<<",\"heat\":"<<a.heat<<",\"cool\":"<<a.cool<<",\"fan\":"<<a.fan<<",\"air\":"<<a.air<<",\"recline\":"<<a.recline<<",\"window\":"<<a.window<<",\"shade\":"<<a.shade<<",\"massage\":"<<a.massage<<'}';}
 std::cout<<"],\"ac\":"<<s.ac<<",\"automatic\":"<<s.automatic<<",\"light\":"<<s.light<<",\"scene\":"<<s.scene<<",\"brightness\":"<<s.brightness<<",\"volume\":"<<s.volume<<",\"playing\":"<<s.playing<<",\"source\":"<<s.source<<",\"output\":"<<s.output<<",\"focus\":"<<s.focus<<",\"roof\":"<<s.roof<<",\"reading\":"<<s.reading<<",\"fresh\":"<<s.fresh<<'}';
}
int main(){Simulator device;Controller controller(device);std::string line;
 while(std::getline(std::cin,line)){
  std::istringstream in(line);int id,seat;std::string key,extra;double value;
  if(!(in>>id>>seat>>key>>value)||(in>>extra)){std::cout<<"{\"error\":\"Malformed protocol request\"}"<<std::endl;continue;}
  auto start=std::chrono::steady_clock::now();Result result=Result::Applied;
  if(key=="connected"&&(value==0||value==1))device.connected=bool(value);
  else if(key!="state")result=controller.dispatch(seat,key,value);
  auto us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();
  std::cout<<"{\"id\":"<<id<<",\"status\":\""<<(result==Result::Applied?"applied":result==Result::Invalid?"invalid":"disconnected")<<"\",\"controllerUs\":"<<us<<",\"connected\":"<<device.connected<<",\"state\":";
  stateJson(controller.state());std::cout<<'}'<<std::endl;
 }
}
