#pragma once

// Because C++ doesn't like using
// namespaces that don't exist
namespace geode {
    namespace addresser {}
    namespace cast {}
    namespace utils {}
    namespace helper {}
    namespace op {}
    namespace stream {}
    namespace view {}
}

namespace geode {
    namespace prelude {
        using namespace ::geode;
        using namespace ::geode::addresser;
        using namespace ::geode::cast;
        using namespace ::geode::helper;
        using namespace ::geode::utils;
        using namespace ::geode::op;
        using namespace ::geode::stream;
        using namespace ::geode::view;
    }
}
