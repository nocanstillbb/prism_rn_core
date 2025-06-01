#ifndef PRISM_RN_CORE_PRISM_MODEL_LIST_PROXY_HPP
#define PRISM_RN_CORE_PRISM_MODEL_LIST_PROXY_HPP

#include <jsi/jsi.h>
#include <memory.h>
#include <prism/rn/prismmodelproxy.hpp>
#include <vector>

namespace prism {
namespace rn {


template <typename T>
class PrismModelListProxy : public facebook::jsi::HostObject {
public:
  PrismModelListProxy( std::shared_ptr<std::vector<T>> list = std::make_shared<std::vector<T>>()) : list_(list) {

  }
  ~PrismModelListProxy() override = default;

private:
  std::shared_ptr<std::vector<T>> list_;
};

} // namespace rn
} // namespace prism

#endif // PRISM_RN_CORE_PRISM_MODEL_LIST_PROXY_HPP