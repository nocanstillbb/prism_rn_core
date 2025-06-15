#ifndef PRISM_RN_CORE_PRISM_MODEL_LIST_PROXY_HPP
#define PRISM_RN_CORE_PRISM_MODEL_LIST_PROXY_HPP

#include "prismLog.h"
#include "prismmodelproxy.hpp"
#include <jsi/jsi.h>
#include <memory.h>
#include <memory>
#include <vector>

namespace prism
{
namespace rn
{

template <typename T> class PrismModelListProxy : public facebook::jsi::HostObject
{
  public:
    using value_type = T;
    // Constructor with default empty list
    PrismModelListProxy(std::shared_ptr<std::vector<std::shared_ptr<prism::rn::PrismModelProxy<T>>>> list = std::make_shared<std::vector<std::shared_ptr<prism::rn::PrismModelProxy<T>>>>())
        : list_(list)
    {
    }

    // Destructor
    ~PrismModelListProxy() override = default;

    // list setter and getter
    std::shared_ptr<std::vector<std::shared_ptr<prism::rn::PrismModelProxy<T>>>> list() const
    {
        return list_;
    }
    void setList(std::shared_ptr<std::vector<std::shared_ptr<prism::rn::PrismModelProxy<T>>>> list)
    {
        list_ = list;
    }

    size_t size() const
    {
        return list_->size();
    }
    // Override getProperty to expose properties to JavaScript
    facebook::jsi::Value get(facebook::jsi::Runtime &rt, const facebook::jsi::PropNameID &name) override
    {
        //#ifdef DEBUG
        //        auto jsinvoke = prism::Container::get()->resolve_object<facebook::react::CallInvoker>();
        //#endif
        std::string propName = name.utf8(rt);
        if (propName == "list")
        {
            facebook::jsi::Array arr = facebook::jsi::Array(rt, list_->size());

            for (size_t i = 0; i < list_->size(); ++i)
            {
                arr.setValueAtIndex(rt, i, facebook::jsi::Object::createFromHostObject(rt, list_->at(i)));
            }
            return arr;
        }
        else if (propName == "length")
        {
            return facebook::jsi::Value(static_cast<int>(list_->size()));
        }
        else
        {
            // Handle other properties or methods if needed
            return facebook::jsi::Value::undefined();
        }
    }

    std::vector<facebook::jsi::PropNameID> getPropertyNames(facebook::jsi::Runtime &rt) override
    {
        std::vector<facebook::jsi::PropNameID> propertyNames;
        propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string("length")));
        propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string("list")));
        return propertyNames;
    }

  private:
    std::shared_ptr<std::vector<std::shared_ptr<prism::rn::PrismModelProxy<T>>>> list_;
};

} // namespace rn
} // namespace prism

#endif // PRISM_RN_CORE_PRISM_MODEL_LIST_PROXY_HPP