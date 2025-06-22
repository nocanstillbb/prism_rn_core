#ifndef PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP
#define PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP

#include "ReactCommon/CallInvoker.h"
#include "prism/rn/prismLog.h"
#include "prism/rn/prismmodellistproxy.hpp"
#include "prism/utilities/typeName.hpp"
#include <iostream>
#include <jsi/jsi.h>
#include <memory>
#include <ostream>
#include <prism/container.hpp>
#include <prism/prism.hpp>
#include <string>

namespace prism
{
namespace rn
{

template <typename T> class PrismModelProxy;
template <typename T> class PrismModelListProxy;

} // namespace rn
} // namespace prism

namespace prism
{
namespace utilities
{

// 提取PrismModelProxy的类型
template <typename T> struct extractPrismModelProxyType
{
    using type = T; // 默认就是原类型
};

template <typename U> struct extractPrismModelProxyType<::prism::rn::PrismModelProxy<U>>
{
    using type = U;
};

// 提最PrismModelListProxy的类型
template <typename T> struct extractPrismModelListProxyType
{
    using type = T; // 默认就是原类型
};

template <typename U> struct extractPrismModelListProxyType<::prism::rn::PrismModelListProxy<U>>
{
    using type = U;
};

} // namespace utilities
} // namespace prism

namespace prism
{
namespace rn
{

template <typename T> class PrismModelProxy : public facebook::jsi::HostObject
{
  public:
    using value_type = T;
    PrismModelProxy(std::shared_ptr<T> instance = std::make_shared<T>()) : instance_(instance)
    {
    }

    template <typename... Args> PrismModelProxy(Args &&...args) : instance_(std::make_shared<T>(std::forward<Args>(args)...))
    {
    }
    ~PrismModelProxy() override = default;

    // Getter for instance_
    std::shared_ptr<T> instance() const
    {
        return instance_;
    }
    void setInstance(std::shared_ptr<T> instance)
    {
        instance_ = instance;
    }

    // Override getProperty to expose properties to JavaScript
    facebook::jsi::Value get(facebook::jsi::Runtime &rt, const facebook::jsi::PropNameID &name) override
    {
        std::string propName = name.utf8(rt);
        bool hasField = false;

        //#ifdef DEBUG
        //        auto jsinvoke = prism::Container::get()->resolve_object<facebook::react::CallInvoker>();
        //#endif

        facebook::jsi::Value result;
        using namespace prism::reflection;
        field_do(*this->instance_, propName.c_str(),
                 [&](auto &&field)
                 {
                     using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
                     // std::shared_ptr<struct> 创建prismmodelproxy 返回object给jsx
                     if constexpr (prism::utilities::is_specialization<FieldType, std::shared_ptr>::value)
                     {
                         using shareT = prism::utilities::extract_shared_ptr_type<FieldType>::type;
                         if constexpr (prism::reflection::has_md<shareT>())
                         {
                             result = ::facebook::jsi::Object::createFromHostObject(rt, std::make_shared<prism::rn::PrismModelProxy<shareT>>(field));
                         }
                         else if constexpr (prism::utilities::is_specialization<shareT, ::prism::rn::PrismModelProxy>::value)
                         {
                             using proxyT = prism::utilities::extractPrismModelProxyType<shareT>::type;
                             if constexpr (prism::reflection::has_md<proxyT>())
                             {
                                 result = ::facebook::jsi::Object::createFromHostObject(rt, field);
                             }
                         }
                         else if constexpr (prism::utilities::is_specialization<shareT, ::prism::rn::PrismModelListProxy>::value)
                         {
                             // using proxyT = prism::utilities::extractPrismModelListProxyType<shareT>::type;
                             result = ::facebook::jsi::Array::createFromHostObject(rt, field);
                         }
                     }
                     else if constexpr (std::is_same_v<FieldType, std::string>)
                     {
                         result = facebook::jsi::String::createFromUtf8(rt, field);
                     }
                     else
                     {
                         result = facebook::jsi::Value(std::forward<decltype(field)>(field));
                     }
                     hasField = true;
                 });
        if (!hasField)
        {
            if (propName == "uuid")
            {
                result = facebook::jsi::String::createFromUtf8(rt, std::to_string(reinterpret_cast<long long>(this)));
                return result;
            }
            return facebook::jsi::Value::undefined();
        }
        else
            return result;
    }

    template <typename TT> void remove_pointer_set(TT &prismobj, facebook::jsi::Runtime &rt, std::string &name, const facebook::jsi::Value &value)
    {
        std::string &propName = name;

        using FieldType = std::remove_reference_t<std::remove_reference_t<TT>>;
        if constexpr (prism::utilities::is_specialization<FieldType, std::shared_ptr>::value || prism::utilities::is_specialization<FieldType, std::unique_ptr>::value ||
                      prism::utilities::is_specialization<FieldType, std::weak_ptr>::value || std::is_pointer_v<FieldType>)
        {
            remove_pointer_set(*prismobj, rt, name, value);
            return;
        }

        if (value.isUndefined())
        {
            // Handle undefined value
        }
        else if (value.isNull())
        {
            // Handle null value
            prism::reflection::field_do(prismobj, propName.c_str(),
                                        [&](auto &&field)
                                        {
                                            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
                                            if constexpr (prism::utilities::is_specialization<FieldType, std::shared_ptr>::value ||
                                                          prism::utilities::is_specialization<FieldType, std::unique_ptr>::value ||
                                                          prism::utilities::is_specialization<FieldType, std::weak_ptr>::value || std::is_pointer_v<FieldType>)
                                            {
                                                field = nullptr;
                                            }
                                        });
        }
        else if (value.isBool())
        {
            // Use boolValue
            bool boolValue = value.getBool();
            prism::reflection::field_do(prismobj, propName.c_str(),
                                        [&](auto &&field)
                                        {
                                            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
                                            if constexpr (std::is_same_v<FieldType, bool>)
                                            {
                                                field = boolValue;
                                            }
                                        });
        }
        else if (value.isNumber())
        {
            // Use numberValue
            double numberValue = value.getNumber();
            prism::reflection::field_do(prismobj, propName.c_str(),
                                        [&](auto &&field)
                                        {
                                            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
                                            if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float> || std::is_same_v<FieldType, int>)
                                            {
                                                field = numberValue;
                                            }
                                        });
        }
        else if (value.isString())
        {
            // Use stringValue
            std::string stringValue = value.getString(rt).utf8(rt);
            prism::reflection::field_do(prismobj, propName.c_str(),
                                        [&](auto &&field)
                                        {
                                            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
                                            if constexpr (std::is_same_v<FieldType, std::string>)
                                            {
                                                field = stringValue;
                                            }
                                        });
        }
        else if (value.isObject())
        {
            // Use objectValue
            facebook::jsi::Object objectValue = value.getObject(rt);
            prism::reflection::field_do(prismobj, propName.c_str(), [&](auto &&field) { remove_pointer_set(field, rt, propName, value); });
        }
        else
        {
            // Handle unexpected type
        }
        // Handle property assignment logic here
    }

    // Override setProperty to handle property assignments from JavaScript
    void set(facebook::jsi::Runtime &rt, const facebook::jsi::PropNameID &name, const facebook::jsi::Value &value) override
    {
        std::string propName = name.utf8(rt);
        if (propName == "notifyUI")
            this->notifyui_ = std::make_shared<jsi::Function>(value.asObject(rt).asFunction(rt));
        else
            remove_pointer_set(*this->instance_, rt, propName, value);
    }

    std::vector<facebook::jsi::PropNameID> getPropertyNames(facebook::jsi::Runtime &rt) override
    {
        std::vector<facebook::jsi::PropNameID> propertyNames;
        propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string("uuid")));
        prism::reflection::for_each_fields(*this->instance_, [&](const char *fname, auto &&_) { propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string(fname))); });
        return propertyNames;
    }

    void notifyUi(jsi::Runtime &rt)
    {
        if (!notifyui_)
            return;
        auto r = ::facebook::jsi::Object::createFromHostObject(rt, std::make_shared<prism::rn::PrismModelProxy<T>>(this->instance_));
        notifyui_->call(rt, r);
    }

  private:
    std::shared_ptr<jsi::Function> notifyui_ = nullptr;
    std::shared_ptr<T> instance_;
};

} // namespace rn
} // namespace prism

#endif // PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP
