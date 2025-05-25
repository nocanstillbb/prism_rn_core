#ifndef PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP
#define PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP

#include <jsi/jsi.h>
#include <memory>
#include <prism/prism.hpp>

namespace prism {
namespace rn {

template <typename T>
class PrismModelProxy : public facebook::jsi::HostObject {
public:
    PrismModelProxy(facebook::jsi::Runtime& runtime,std::shared_ptr<T> instance):instance_(instance)
    {

    }
    ~PrismModelProxy() override = default;

    // Override getProperty to expose properties to JavaScript
    facebook::jsi::Value get( facebook::jsi::Runtime& rt, const facebook::jsi::PropNameID& name) override 
    {
        std::string propName = name.utf8(rt);
        bool hasField = false;

        facebook::jsi::Value result;
        prism::reflection::field_do(*this->instance_, propName.c_str(), [&](auto&& field) {
            //using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
            result = field;
            hasField = true;
            
        });
        if(!hasField)
            return facebook::jsi::Value::undefined();
        else
            return result;;
    }


    template <typename TT>
    void remove_pointer_set(TT& prismobj,facebook::jsi::Runtime& rt, std::string& name, const facebook::jsi::Value& value)
    {
        using FieldType = std::remove_reference_t<std::remove_reference_t<TT>>;
        if constexpr (prism::utilities::is_specialization<FieldType, std::shared_ptr>::value||
                      prism::utilities::is_specialization<FieldType, std::unique_ptr>::value||
                      prism::utilities::is_specialization<FieldType, std::weak_ptr>::value||
                      std::is_pointer_v<FieldType>)
        {
            remove_pointer_set(*prismobj, rt, name, value);
            return;
        }

        std::string& propName = name;

        if (value.isUndefined()) {
            // Handle undefined value
        } else if (value.isNull()) {
            // Handle null value
            prism::reflection::field_do(prismobj, propName.c_str(), [&](auto&& field) {
            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
            if constexpr (prism::utilities::is_specialization<FieldType, std::shared_ptr>::value ||
                      prism::utilities::is_specialization<FieldType, std::unique_ptr>::value ||
                      prism::utilities::is_specialization<FieldType, std::weak_ptr>::value ||
                      std::is_pointer_v<FieldType>) {
                field = nullptr;
            }
            });
        } else if (value.isBool()) {
            // Use boolValue
            bool boolValue = value.getBool();
            prism::reflection::field_do(prismobj, propName.c_str(), [&](auto&& field) {
            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
            if constexpr (std::is_same_v<FieldType, bool>) {
                field = boolValue;
            }
            });
        } else if (value.isNumber()) {
            // Use numberValue
            double numberValue = value.getNumber();
            prism::reflection::field_do(prismobj, propName.c_str(), [&](auto&& field) {
            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
            if constexpr (std::is_same_v<FieldType, double> ||
                      std::is_same_v<FieldType, float> ||
                      std::is_same_v<FieldType, int>) {
                field = numberValue;
            }
            });
        } else if (value.isString()) {
            // Use stringValue
            std::string stringValue = value.getString(rt).utf8(rt);
            prism::reflection::field_do(prismobj, propName.c_str(), [&](auto&& field) {
            using FieldType = std::remove_reference_t<std::remove_reference_t<decltype(field)>>;
            if constexpr (std::is_same_v<FieldType, std::string>) {
                field = stringValue;
            }
            });
        } else if (value.isObject()) {
            // Use objectValue
            facebook::jsi::Object objectValue = value.getObject(rt);
            prism::reflection::field_do(prismobj, propName.c_str(), [&](auto&& field) {
            remove_pointer_set(field, rt, propName, value);
            });
        } else {
            // Handle unexpected type
        }
        // Handle property assignment logic here
    }

    // Override setProperty to handle property assignments from JavaScript
    void set( facebook::jsi::Runtime& rt, const facebook::jsi::PropNameID& name, const facebook::jsi::Value& value) override {
        std::string propName = name.utf8(rt);
        remove_pointer_set(*this->instance_,rt, propName, value);
    }

    std::vector<facebook::jsi::PropNameID> getPropertyNames( facebook::jsi::Runtime& rt) override 
    { 
        std::vector<facebook::jsi::PropNameID> propertyNames;
        prism::reflection::for_each_fields(*this->instance_ ,[&](const char* fname,auto&& _) {
            propertyNames.push_back(facebook::jsi::PropNameID::forUtf8(rt, std::string(fname)));
        });
        return propertyNames;
    }

private:
    std::shared_ptr<T> instance_;
};

} // namespace rn
} // namespace prism

#endif // PRISM_RN_CORE_PRISM_MODEL_PROXY_HPP
