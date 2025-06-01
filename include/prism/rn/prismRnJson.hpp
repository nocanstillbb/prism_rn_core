#ifndef PRISM_RN_CORE_HPP_PRISMRN_JSON_HPP
#define PRISM_RN_CORE_HPP_PRISMRN_JSON_HPP

#include "prismmodelproxy.hpp"
#include <jsi/jsi.h>
#include <prism/prismJson.hpp>
#include <type_traits>

namespace prism::json::privates
{

// prismModelProxy
template <class T>
struct jsonObject<T, std::enable_if_t<prism::utilities::is_specialization<T, prism::rn::PrismModelProxy>::value, void>>
    : public jsonObjectBase<jsonObject<T>>
{

    using jsonObjectBase<jsonObject<T>>::alias_map_;
    constexpr static void append_sub_kvs([[maybe_unused]] std::ostringstream &stream,
                                         [[maybe_unused]] const char *fname, [[maybe_unused]] T &&value,
                                         [[maybe_unused]] int identation, [[maybe_unused]] int &&level)
    {
        using valueType_t = std::remove_reference_t<std::remove_reference_t<decltype(*value.instance())>>;
        jsonObject<valueType_t>::append_sub_kvs(stream, fname, std::move(*value.instance()), identation,
                                                std::move(level));
    }

    static void read_sub_kv([[maybe_unused]] T &&model, [[maybe_unused]] std::string_view &&str,
                            [[maybe_unused]] int kstart, [[maybe_unused]] int kend, [[maybe_unused]] int vstart,
                            [[maybe_unused]] int vend)
    {
        using valueType_t = std::remove_reference_t<std::remove_reference_t<decltype(*model.instance())>>;
        if (!model.instance())
            model.setInstance(std::make_shared<valueType_t>());
        jsonObject<valueType_t>::read_sub_kv(std::move(*model.instance()), std::move(str), kstart, kend, vstart, vend);
    }
};

} // namespace prism::json::privates

#endif // PRISM_RN_CORE_HPP_PRISMRN_JSON_HPP
