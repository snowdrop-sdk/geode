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
