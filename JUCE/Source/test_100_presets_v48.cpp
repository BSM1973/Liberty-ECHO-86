#include "FactoryPresets.h"
#include <cassert>
#include <cmath>
#include <set>
#include <string>
#include <iostream>
int main(){
 static_assert(echo86::factoryPresets.size()==100);
 static_assert(echo86::parameterIds.size()==24);
 std::set<std::string> names;
 std::set<std::string> values;
 for(const auto& p:echo86::factoryPresets){
  assert(names.insert(p.name).second);
  std::string fingerprint;
  for(float x:p.values){assert(std::isfinite(x));fingerprint+=std::to_string(x)+",";}
  assert(values.insert(fingerprint).second);
 }
 std::cout<<"PASS: 100 unique names, 100 unique 24-parameter sets\n";
}
