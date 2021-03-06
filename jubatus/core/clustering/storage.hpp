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

#ifndef JUBATUS_CORE_CLUSTERING_STORAGE_HPP_
#define JUBATUS_CORE_CLUSTERING_STORAGE_HPP_

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <msgpack.hpp>
#include "../framework/mixable_helper.hpp"
#include "../common/version.hpp"
#include "clustering_config.hpp"
#include "types.hpp"
#include "event_dispatcher.hpp"

namespace jubatus {
namespace core {
namespace clustering {

enum storage_event_type {
  REVISION_CHANGE,
  UPDATE
};

class storage : public event_dispatcher<storage_event_type, wplist> {
 public:
  storage(const std::string& name, const clustering_config& config);
  virtual ~storage() {}

  void get_diff(diff_t& d) const;
  bool put_diff(const diff_t& d);
  void mix(const diff_t&, diff_t& ret);

  size_t get_revision();

  virtual void add(const weighted_point& point) = 0;
  virtual wplist get_mine() const = 0;

  virtual wplist get_all() const;
  virtual wplist get_common() const;

  core::storage::version get_version() const {
    return core::storage::version();
  }

  void pack(framework::packer& packer) const;
  void unpack(msgpack::object o);
  void clear();

  MSGPACK_DEFINE(revision_, name_, config_, common_);

 protected:
  void increment_revision();
  virtual void pack_impl_(framework::packer& packer) const;
  virtual void unpack_impl_(msgpack::object o);
  virtual void clear_impl_();

  size_t revision_;
  std::string name_;
  clustering_config config_;

  std::vector<std::pair<std::string, wplist> > common_;
};

typedef framework::linear_mixable_helper<storage, diff_t> mixable_storage;

}  // namespace clustering
}  // namespace core
}  // namespace jubatus

#endif  // JUBATUS_CORE_CLUSTERING_STORAGE_HPP_
