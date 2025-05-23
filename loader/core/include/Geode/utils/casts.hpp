#pragma once

#include <inttypes.h>
#include <iostream>
#include <string>
#include <type_traits>

namespace geode::cast {
    /**
     * Alias for static_cast
     */
    template <class T, class F>
    static constexpr T as(F const v) {
        return static_cast<T>(v);
    }

    /**
     * Cast from anything to anything else,
     * provided they are the same size
     */
    template <class T, class F>
    static constexpr T union_cast(F const v) {
        static_assert(sizeof(F) == sizeof(T), "union_cast: R and T don't match in size!");

        union {
            T t;
            F f;
        } x;

        x.f = v;
        return x.t;
    }

    /**
     * Reference casting. Does a pointer-to-pointer
     * cast but uses reference syntactic sugar to
     * look cleaner.
     */
    template <class T, class F>
    static constexpr T reference_cast(F v) {
        return reinterpret_cast<T&>(v);
    }

    /**
     * Cast based on RTTI. Casts an adjusted this pointer
     * to it's non offset form.
     */
    template <class T, class F>
    static constexpr T base_cast(F const obj) {
        return static_cast<T>(dynamic_cast<void*>(obj));
    }

    /**
     * Cast based on RTTI. This is used to check
     * if an object is exactly the class needed. Returns
     * nullptr on failure.
     */
    template <class T, class F>
    static T exact_cast(F const obj) {
        if (std::strcmp(typeid(*obj).name(), typeid(std::remove_pointer_t<T>).name()) == 0) {
            return base_cast<T>(obj);
        }
        return nullptr;
    }

    /**
     * Shared pointer cast equivalent to typeinfo_cast.
     */
    template<class T, class U>
    std::shared_ptr<T> typeinfo_pointer_cast(std::shared_ptr<U> const& r) noexcept {
        // https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
        auto p = typeinfo_cast<typename std::shared_ptr<T>::element_type*>(r.get());
        return std::shared_ptr<T>(r, p);
    }
}
