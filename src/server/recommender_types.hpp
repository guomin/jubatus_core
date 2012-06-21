
// This file is auto-generated from recommender.idl
// *** DO NOT EDIT ***

#ifndef RECOMMENDER_TYPES_HPP_
#define RECOMMENDER_TYPES_HPP_


#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <msgpack.hpp>


namespace jubatus {

typedef std::string converter_config;

typedef std::vector<std::pair<std::string, float > > similar_result;

struct config_data {
public:

  
  MSGPACK_DEFINE(method, converter);  

  std::string method;
  converter_config converter;
};

struct datum {
public:

  
  MSGPACK_DEFINE(string_values, num_values);  

  std::vector<std::pair<std::string, std::string > > string_values;
  std::vector<std::pair<std::string, double > > num_values;
};

} // namespace jubatus



#endif // RECOMMENDER_TYPES_HPP_
