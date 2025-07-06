#ifndef PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP
#define PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP

#include "ReactCommon/CallInvoker.h"
#include "include/prism/rn/prismLog.h"
#include "prism/rn/prismmodellistproxy.hpp"
#include "prism/utilities/typeName.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <jsi/jsi.h>
#include <map>
#include <memory>
#include <ostream>
#include <prism/container.hpp>
#include <prism/prism.hpp>
#include <string>
#include <tuple>
#include <type_traits>

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

        // auto jsinvoke = prism::Container::get()->resolve_object<facebook::react::CallInvoker>();

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
            else if (propName == "register")
            {
                return jsi::Function::createFromHostFunction(rt,   // runtime
                                                             name, //函数名
                                                             2,    // 参数个数
                                                             [this](jsi::Runtime &rt, const jsi::Value &thisValue, const jsi::Value *args, size_t count) -> jsi::Value
                                                             {
                                                                 if (count >= 2 && args[0].isString() && args[1].isObject())
                                                                 {
                                                                     auto fname = args[0].toString(rt);
                                                                     std::shared_ptr<jsi::Function> func = std::make_shared<jsi::Function>(args[1].asObject(rt).asFunction(rt));
                                                                     if (func)
                                                                     {
                                                                         size_t int_key = reinterpret_cast<size_t>(func.get());
                                                                         std::string key = std::to_string(int_key);
                                                                         notifyui_[key] = std::make_tuple(fname.utf8(rt), func);
                                                                         // LOG_INFO_F(rt, jsinvoke, "after registe map size:{}", notifyui_.size());
                                                                         return jsi::String::createFromUtf8(rt, key);
                                                                     }
                                                                     return jsi::String::createFromUtf8(rt, "");
                                                                 }
                                                                 else
                                                                 {
                                                                     return jsi::String::createFromUtf8(rt, "");
                                                                 }
                                                             });
            }
            else if (propName == "unregister")
            {
                return jsi::Function::createFromHostFunction(rt,   // runtime
                                                             name, //函数名
                                                             1,    // 参数个数
                                                             [this](jsi::Runtime &rt, const jsi::Value &thisValue, const jsi::Value *args, size_t count) -> jsi::Value
                                                             {
                                                                 if (count >= 1 && args[0].isString())
                                                                 {
                                                                     auto key = args[0].toString(rt).utf8(rt);
                                                                     notifyui_.erase(key);
                                                                     // LOG_INFO_F(rt, jsinvoke, "after unregiste map size:{}", notifyui_.size());
                                                                     return args[0].toString(rt);
                                                                 }
                                                                 else
                                                                 {
                                                                     // not found return undefine
                                                                     return jsi::String::createFromUtf8(rt, "");
                                                                 }
                                                             });
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
                                                if (field != nullptr)
                                                {
                                                    field = nullptr;
                                                    this->notifyUi(&rt, name);
                                                }
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
                                                if (field != boolValue)
                                                {
                                                    field = boolValue;
                                                    this->notifyUi(&rt, name);
                                                }
                                            }
                                        });
        }
        else if (value.isNumber())
        {
            auto invoker = prism::Container::get()->resolve_object<facebook::react::CallInvoker>();

            LOG_INFO_F(rt, invoker, "111111");
            // Use numberValue
            double numberValue = value.getNumber();
            prism::reflection::field_do(prismobj, propName.c_str(),
                                        [&](auto &&field)
                                        {
                                            LOG_INFO_F(rt, invoker, "111112");
                                            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
                                            if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float> || std::is_same_v<FieldType, int>)
                                            {
                                                LOG_INFO_F(rt, invoker, "111113,left:{},right{}", field, numberValue);
                                                if (std::abs(field - numberValue) >= 0.000001)
                                                {
                                                    LOG_INFO_F(rt, invoker, "111114");
                                                    field = numberValue;
                                                    this->notifyUi(&rt, name);
                                                }
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
                                                if (field != stringValue)
                                                {
                                                    field = stringValue;
                                                    this->notifyUi(&rt, name);
                                                }
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
        remove_pointer_set(*this->instance_, rt, propName, value);
    }

    std::vector<facebook::jsi::PropNameID> getPropertyNames(facebook::jsi::Runtime &rt) override
    {
        std::vector<facebook::jsi::PropNameID> propertyNames;
        propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string("uuid")));
        prism::reflection::for_each_fields(*this->instance_, [&](const char *fname, auto &&_) { propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string(fname))); });
        return propertyNames;
    }

    void notifyUi(jsi::Runtime *rt)
    {
        if (!notifyui_.size())
            return;
        if (!rt)
            return;
        // auto r = ::facebook::jsi::Object::createFromHostObject(*rt, std::make_shared<prism::rn::PrismModelProxy<T>>(this->instance_));
        for (auto [k, v] : notifyui_)
        {
            // k : pointer adderess str
            // v : tuple<fname,js_function>
            std::string fname = std::get<0>(v);
            prism::reflection::field_do(*this->instance_, fname.c_str(), [&](auto &&value) { std::get<1>(v)->call(*rt, value); });
        }
    }
    void notifyUi(jsi::Runtime *rt, std::string field_name)
    {
        if (!notifyui_.size())
            return;
        if (!rt)
            return;
        // auto r = ::facebook::jsi::Object::createFromHostObject(*rt, std::make_shared<prism::rn::PrismModelProxy<T>>(this->instance_));
        for (auto [k, v] : notifyui_)
        {
            // k : pointer adderess str
            // v : tuple<fname,js_function>
            std::string fname = std::get<0>(v);
            if (field_name == fname)
                prism::reflection::field_do(*this->instance_, fname.c_str(),
                                            [&](auto &&value)
                                            {
                                                using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(value)>>;

                                                if constexpr (!(prism::utilities::is_specialization<FieldType, std::shared_ptr>::value ||
                                                                prism::utilities::is_specialization<FieldType, std::unique_ptr>::value ||
                                                                prism::utilities::is_specialization<FieldType, std::weak_ptr>::value || std::is_pointer_v<FieldType>)&&!std::is_class_v<FieldType>)

                                                    std::get<1>(v)->call(*rt, value);
                                            });
        }
    }

  private:
    std::map<std::string, std::tuple<std::string, std::shared_ptr<jsi::Function>>> notifyui_;
    std::shared_ptr<T> instance_;
};

} // namespace rn
} // namespace prism

#endif // PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP
