#pragma once

#if !defined(GEODE_CONCAT)
    #define GEODE_WRAPPER_CONCAT(x, y) x##y
    #define GEODE_CONCAT(x, y) GEODE_WRAPPER_CONCAT(x, y)
#endif

#if !defined(GEODE_STR)
    #define GEODE_WRAPPER_STR(...) #__VA_ARGS__
    #define GEODE_STR(...) GEODE_WRAPPER_STR(__VA_ARGS__)
#endif

#if !defined(GEODE_EXPAND)
    #define GEODE_EXPAND(x) x
#endif

#if !defined(GEODE_INVOKE)
    #define GEODE_INVOKE(macro, ...) GEODE_EXPAND(macro(__VA_ARGS__))
#endif


#if !defined($execute)
    #define $execute                                                                                  \
        template <class>                                                                              \
        void GEODE_CONCAT(geodeExecFunction, __LINE__)();                                             \
        namespace {                                                                                   \
            struct GEODE_CONCAT(ExecFuncUnique, __LINE__) {};                                         \
        }                                                                                             \
        static inline auto GEODE_CONCAT(Exec, __LINE__) =                                             \
            (GEODE_CONCAT(geodeExecFunction, __LINE__) < GEODE_CONCAT(ExecFuncUnique, __LINE__) > (), \
            0);                                                                                      \
        template <class>                                                                              \
        void GEODE_CONCAT(geodeExecFunction, __LINE__)()
#endif

#define GEODE_FORWARD_COMPAT_DISABLE_HOOKS_INNER(message) \
    if (Loader::get()->isForwardCompatMode()) {           \
        if (strlen(message)) {                            \
            log::warn("[Forward Compat] " message);       \
        }                                                 \
        for (const auto& [_, hook] : self.m_hooks) {      \
            hook->setAutoEnable(false);                   \
        }                                                 \
    }
#define GEODE_FORWARD_COMPAT_ENABLE_HOOKS_INNER(message)  \
    if (!Loader::get()->isForwardCompatMode()) {          \
        if (strlen(message)) {                            \
            log::warn("[Forward Compat] " message);       \
        }                                                 \
        for (const auto& [_, hook] : self.m_hooks) {      \
            hook->setAutoEnable(false);                   \
        }                                                 \
    }
#define GEODE_FORWARD_COMPAT_DISABLE_HOOKS(message)       \
    static void onModify(const auto& self) {              \
        GEODE_FORWARD_COMPAT_DISABLE_HOOKS_INNER(message) \
    }
#define GEODE_FORWARD_COMPAT_ENABLE_HOOKS(message)        \
    static void onModify(const auto& self) {              \
        GEODE_FORWARD_COMPAT_ENABLE_HOOKS_INNER(message)  \
    }



namespace geode {
    struct ZeroConstructorType {};

    static constexpr auto ZeroConstructor = ZeroConstructorType();

    struct CutoffConstructorType {};

    static constexpr auto CutoffConstructor = CutoffConstructorType();
}

#define GEODE_CUSTOM_CONSTRUCTOR_BEGIN(Class_) \
    GEODE_ZERO_CONSTRUCTOR_BEGIN(Class_)       \
    GEODE_CUTOFF_CONSTRUCTOR_BEGIN(Class_)

#define GEODE_CUSTOM_CONSTRUCTOR_COCOS(Class_, Base_) \
    GEODE_ZERO_CONSTRUCTOR(Class_, Base_)             \
    GEODE_CUTOFF_CONSTRUCTOR_COCOS(Class_, Base_)

#define GEODE_CUSTOM_CONSTRUCTOR_GD(Class_, Base_) \
    GEODE_ZERO_CONSTRUCTOR(Class_, Base_)          \
    GEODE_CUTOFF_CONSTRUCTOR_GD(Class_, Base_)

#define GEODE_CUSTOM_CONSTRUCTOR_CUTOFF(Class_, Base_) \
    GEODE_ZERO_CONSTRUCTOR(Class_, Base_)              \
    GEODE_CUTOFF_CONSTRUCTOR_CUTOFF(Class_, Base_)

#define GEODE_ZERO_CONSTRUCTOR_BEGIN(Class_)                                              \
    Class_(geode::ZeroConstructorType, void*) {}                                          \
    Class_(geode::ZeroConstructorType, size_t fill) :                                     \
        Class_(geode::ZeroConstructor, std::memset(static_cast<void*>(this), 0, fill)) {} \
    Class_(geode::ZeroConstructorType) : Class_(geode::ZeroConstructor, nullptr) {}

#define GEODE_ZERO_CONSTRUCTOR(Class_, Base_)                                                \
    Class_(geode::ZeroConstructorType, size_t fill) : Base_(geode::ZeroConstructor, fill) {} \
    Class_(geode::ZeroConstructorType) : Base_(geode::ZeroConstructor, sizeof(Class_)) {}

#define GEODE_FILL_CONSTRUCTOR(Class_, Offset_)                                          \
    Class_(geode::CutoffConstructorType, size_t fill) :                                  \
        Class_(                                                                          \
            geode::CutoffConstructor,                                                    \
            std::memset(reinterpret_cast<std::byte*>(this) + Offset_, 0, fill - Offset_) \
        ) {}                                                                             \
    Class_(geode::CutoffConstructorType, void*)

#define GEODE_CUTOFF_CONSTRUCTOR_BEGIN(Class_)                      \
    GEODE_MACOS(GEODE_FILL_CONSTRUCTOR(Class_, 0){})                \
    GEODE_IOS(GEODE_FILL_CONSTRUCTOR(Class_, 0){})                  \
    GEODE_WINDOWS(Class_(geode::CutoffConstructorType, size_t fill) \
                    : Class_() {})                                    \
    GEODE_ANDROID(GEODE_FILL_CONSTRUCTOR(Class_, 0){})

#define GEODE_CUTOFF_CONSTRUCTOR_COCOS(Class_, Base_)               \
    GEODE_MACOS(Class_(geode::CutoffConstructorType, size_t fill)   \
                : Base_(geode::CutoffConstructor, fill){})          \
    GEODE_IOS(Class_(geode::CutoffConstructorType, size_t fill)     \
                : Base_(geode::CutoffConstructor, fill){})            \
    GEODE_WINDOWS(Class_(geode::CutoffConstructorType, size_t fill) \
                    : Class_() {})                                    \
    GEODE_ANDROID(Class_(geode::CutoffConstructorType, size_t fill)   \
                : Base_(geode::CutoffConstructor, fill){})

#define GEODE_CUTOFF_CONSTRUCTOR_GD(Class_, Base_)                  \
    GEODE_WINDOWS(Class_(geode::CutoffConstructorType, size_t fill) \
                    : Base_(geode::CutoffConstructor, fill){})        \
    GEODE_MACOS(Class_(geode::CutoffConstructorType, size_t fill)   \
                : Base_(geode::CutoffConstructor, fill){})          \
    GEODE_IOS(Class_(geode::CutoffConstructorType, size_t fill)     \
                : Base_(geode::CutoffConstructor, fill){})            \
    GEODE_ANDROID(Class_(geode::CutoffConstructorType, size_t fill) \
                : Base_(geode::CutoffConstructor, fill){})

#define GEODE_CUTOFF_CONSTRUCTOR_CUTOFF(Class_, Base_)                       \
    GEODE_WINDOWS(GEODE_FILL_CONSTRUCTOR(Class_, sizeof(Base_)) : Base_(){}) \
    GEODE_ANDROID(Class_(geode::CutoffConstructorType, size_t fill)          \
                : Base_(geode::CutoffConstructor, fill){})                   \
    GEODE_MACOS(Class_(geode::CutoffConstructorType, size_t fill)            \
                : Base_(geode::CutoffConstructor, fill){})                   \
    GEODE_IOS(Class_(geode::CutoffConstructorType, size_t fill)              \
                : Base_(geode::CutoffConstructor, fill){})