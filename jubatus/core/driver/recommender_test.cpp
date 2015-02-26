// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2013 Preferred Networks and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <vector>
#include <utility>
#include <map>
#include <string>

#include <gtest/gtest.h>

#include "jubatus/util/lang/cast.h"
#include "jubatus/util/lang/shared_ptr.h"
#include "../common/jsonconfig.hpp"

#include "../fv_converter/datum.hpp"
#include "../fv_converter/converter_config.hpp"
#include "../recommender/recommender.hpp"
#include "../recommender/recommender_type.hpp"
#include "../recommender/recommender_factory.hpp"
#include "../classifier/classifier_test_util.hpp"
#include "../framework/stream_writer.hpp"
#include "../unlearner/lru_unlearner.hpp"
#include "recommender.hpp"

#include "test_util.hpp"

using std::string;
using std::map;
using std::vector;
using std::pair;
using std::make_pair;
using std::stringstream;
using jubatus::util::lang::shared_ptr;
using jubatus::util::text::json::json;
using jubatus::util::text::json::json_object;
using jubatus::util::text::json::json_integer;
using jubatus::util::text::json::json_string;
using jubatus::util::text::json::json_float;
using jubatus::util::lang::lexical_cast;
using jubatus::core::fv_converter::datum;
using jubatus::core::recommender::recommender_base;
namespace jubatus {
namespace core {
namespace driver {

class recommender_test : public ::testing::Test {
 protected:
  void SetUp() {
    recommender_.reset(new driver::recommender(
          jubatus::util::lang::shared_ptr<core::recommender::recommender_base>(
            new core::recommender::inverted_index),
          make_fv_converter()));
  }

  void TearDown() {
    recommender_.reset();
  }

  jubatus::util::lang::shared_ptr<core::driver::recommender> recommender_;
};


TEST_F(recommender_test, small) {
  datum d;
  d.num_values_.push_back(make_pair("f1", 1.0));
  recommender_->update_row("key", d);
  recommender_->clear_row("key");
  recommender_->update_row("key", d);

  recommender_->complete_row_from_datum(d);
  recommender_->complete_row_from_id("key");
}

class nn_recommender_test
    : public ::testing::TestWithParam<
        shared_ptr<core::recommender::recommender_base> > {
 protected:
  void SetUp() {
    recommender_.reset(new driver::recommender(
          jubatus::util::lang::shared_ptr<core::recommender::recommender_base>(
            new core::recommender::inverted_index),
          make_tf_idf_fv_converter()));
  }

  void TearDown() {
    recommender_.reset();
  }

  jubatus::util::lang::shared_ptr<core::driver::recommender> recommender_;
};

vector<shared_ptr<recommender_base> >
create_recommender_bases() {
  const std::string id("my_id");
  vector<shared_ptr<recommender_base> > recommenders;

  vector<pair<string, int> > pattern;
  for (size_t i = 8; i < 3000; i = i << 1) {  // up to 2048
    pattern.push_back(make_pair("lsh", i));
    pattern.push_back(make_pair("euclid_lsh", i));
    pattern.push_back(make_pair("minhash", i));
  }
  for (size_t i = 0; i < pattern.size(); ++i) {
    shared_ptr<core::table::column_table> table(new core::table::column_table);

    json jsconf(new json_object);
    json method_param(new json_object);
    method_param["hash_num"] = new json_integer(pattern[i].second);
    jsconf["parameter"] = method_param;
    jsconf["method"] = new json_string(pattern[i].first);
    common::jsonconfig::config conf(jsconf);
    recommenders.push_back(
        core::recommender::recommender_factory::create_recommender(
            "nearest_neighbor_recommender",
            conf,
            id));
  }
  return recommenders;
}

fv_converter::datum create_datum_str(const string& key, const string& value) {
  fv_converter::datum d;
  d.string_values_.push_back(make_pair(key, value));
  return d;
}

TEST_P(nn_recommender_test, update) {
  datum d = create_datum_str("a", "f g h");
  for (int i = 0; i < 10; ++i) {
    recommender_->update_row("id1", create_datum_str("a", "a b c"));
    recommender_->update_row("id2", create_datum_str("a", "d e f"));
    recommender_->update_row("id3", create_datum_str("a", "e f g"));
    recommender_->update_row("id4", create_datum_str("a", "f g h"));
    recommender_->update_row("id5", create_datum_str("a", "h i j"));
    recommender_->update_row("id6", create_datum_str("a", "i j a"));
    recommender_->update_row("id7", create_datum_str("a", "j a b"));
  }

  vector<pair<string, float> > ret =
      recommender_->similar_row_from_datum(d, 10);
  ASSERT_EQ("id4", ret[0].first);
}

INSTANTIATE_TEST_CASE_P(nn_recommender_test_instance,
    nn_recommender_test,
    testing::ValuesIn(create_recommender_bases()));

TEST(inverted_index_unlearner, lru_update) {
  unlearner::lru_unlearner::config conf;
  conf.max_size = 3;
  shared_ptr<unlearner::unlearner_base> unl(new unlearner::lru_unlearner(conf));
  shared_ptr<driver::recommender> recommender =
      shared_ptr<driver::recommender>(new driver::recommender(
        shared_ptr<recommender_base>(
            new core::recommender::inverted_index(unl)),
        make_tf_idf_fv_converter()));
  recommender->update_row("id1", create_datum_str("a", "a b c"));
  recommender->update_row("id2", create_datum_str("a", "d e f"));
  recommender->update_row("id3", create_datum_str("a", "e f g"));
  recommender->update_row("id4", create_datum_str("a", "f g h"));
  recommender->update_row("id5", create_datum_str("a", "h i j"));
  recommender->update_row("id6", create_datum_str("a", "i j a"));
  recommender->update_row("id7", create_datum_str("a", "j a b"));

  vector<pair<string, float> > ret = recommender->similar_row_from_id("id6", 4);
  ASSERT_EQ(3u, ret.size());
}

TEST(inverted_index_unlearner, lru_delete) {
  unlearner::lru_unlearner::config conf;
  conf.max_size = 3;
  shared_ptr<unlearner::unlearner_base> unl(new unlearner::lru_unlearner(conf));
  shared_ptr<driver::recommender> recommender =
      shared_ptr<driver::recommender>(new driver::recommender(
        shared_ptr<recommender_base>(
            new core::recommender::inverted_index(unl)),
        make_tf_idf_fv_converter()));
  recommender->update_row("id1", create_datum_str("a", "a b c"));
  recommender->update_row("id2", create_datum_str("a", "d e f"));
  recommender->update_row("id3", create_datum_str("a", "e f g"));
  recommender->clear_row("id1");
  recommender->update_row("id4", create_datum_str("a", "f g h"));
  recommender->update_row("id5", create_datum_str("a", "h i j"));
  recommender->update_row("id6", create_datum_str("a", "i j a"));
  recommender->update_row("id7", create_datum_str("a", "j a b"));

  vector<pair<string, float> > ret = recommender->similar_row_from_id("id6", 4);
  ASSERT_EQ(3u, ret.size());

  vector<string> all = recommender->get_all_rows();
  ASSERT_EQ(3u, all.size());
}

TEST(inverted_index_unlearner, mix) {
  unlearner::lru_unlearner::config conf;
  conf.max_size = 3;
  shared_ptr<unlearner::unlearner_base> unl1(new unlearner::lru_unlearner(conf));
  shared_ptr<driver::recommender> recommender1 =
      shared_ptr<driver::recommender>(new driver::recommender(
        shared_ptr<recommender_base>(
            new core::recommender::inverted_index(unl1)),
        make_tf_idf_fv_converter()));
  framework::linear_mixable* mixable1 =
      dynamic_cast<framework::linear_mixable*>(recommender1->get_mixable());
  recommender1->update_row("id1", create_datum_str("a", "a b c"));
  recommender1->update_row("id2", create_datum_str("a", "d e f"));
  recommender1->update_row("id3", create_datum_str("a", "e f g"));

  shared_ptr<unlearner::unlearner_base> unl2(new unlearner::lru_unlearner(conf));
  shared_ptr<driver::recommender> recommender2 =
      shared_ptr<driver::recommender>(new driver::recommender(
        shared_ptr<recommender_base>(
            new core::recommender::inverted_index(unl2)),
        make_tf_idf_fv_converter()));
  recommender2->update_row("id2", create_datum_str("a", "d e f"));
  recommender2->update_row("id3", create_datum_str("a", "e f g"));
  recommender2->update_row("id4", create_datum_str("a", "f g h"));
  framework::linear_mixable* mixable2 =
      dynamic_cast<framework::linear_mixable*>(recommender2->get_mixable());

  msgpack::sbuffer data1;
  msgpack::unpacked unpacked1;
  {
    core::framework::stream_writer<msgpack::sbuffer> st(data1);
    core::framework::jubatus_packer jp(st);
    core::framework::packer pk(jp);
    mixable1->get_diff(pk);
    msgpack::unpack(&unpacked1, data1.data(), data1.size());
  }

  msgpack::sbuffer data2;
  msgpack::unpacked unpacked2;
  {
    core::framework::stream_writer<msgpack::sbuffer> st(data2);
    core::framework::jubatus_packer jp(st);
    core::framework::packer pk(jp);
    mixable2->get_diff(pk);
    msgpack::unpack(&unpacked2, data2.data(), data2.size());
  }


  framework::diff_object diff = mixable2->convert_diff_object(unpacked2.get());
  mixable2->mix(unpacked1.get(), diff);

  mixable1->put_diff(diff);
  mixable2->put_diff(diff);

  ASSERT_EQ(3u, recommender1->get_all_rows().size());
  ASSERT_EQ(3u, recommender2->get_all_rows().size());
}

}  // namespace driver
}  // namespace core
}  // namespace jubatus
