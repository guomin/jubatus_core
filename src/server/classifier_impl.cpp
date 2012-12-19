// this program is automatically generated by jenerator. do not edit.
#include "../framework.hpp"
#include "classifier_server.hpp"
#include "classifier_serv.hpp"
using namespace jubatus;
using namespace jubatus::framework;
namespace jubatus { namespace server {
class classifier_impl_ : public classifier<classifier_impl_>
{
public:
  classifier_impl_(const server_argv& a):
    classifier<classifier_impl_>(a.timeout),
    p_(new server_helper<classifier_serv>(a))
  {}

  bool set_config(std::string name, config_data c) //update broadcast
  { JWLOCK__(p_); return get_p()->set_config(c); }

  config_data get_config(std::string name) //analysis random
  { JRLOCK__(p_); return get_p()->get_config(); }

  int train(std::string name, std::vector<std::pair<std::string,datum > > data) //update random
  { JWLOCK__(p_); return get_p()->train(data); }

  std::vector<std::vector<estimate_result > > classify(std::string name, std::vector<datum > data) //analysis random
  { JRLOCK__(p_); return get_p()->classify(data); }

  bool save(std::string name, std::string id) //update broadcast
  { JWLOCK__(p_); return get_p()->save(id); }

  bool load(std::string name, std::string id) //update broadcast
  { JWLOCK__(p_); return get_p()->load(id); }

  std::map<std::string,std::map<std::string,std::string > > get_status(std::string name) //analysis broadcast
  { JRLOCK__(p_); return p_->get_status(); }
  int run(){ return p_->start(*this); };
  common::cshared_ptr<classifier_serv> get_p(){ return p_->server(); };
private:
  common::cshared_ptr<server_helper<classifier_serv> > p_;
};
}} // namespace jubatus::server
int main(int args, char** argv){
  return
    jubatus::framework::run_server<jubatus::server::classifier_impl_>
       (args, argv, "classifier");
}
