#include "about.hpp"

geode::VersionInfo about::getLoaderVersion() {
    return {
        4,
        3,
        1,
        std::nullopt
    };
}
const char* about::getLoaderVersionStr() { return "4.3.1"; }
const char* about::getLoaderCommitHash() { return "426b1d1d-dirty"; }
const char* about::getBindingsCommitHash() { return "a3f99680"; }
const char* about::getLoaderModJson() { return R"JSON_SEPARATOR()JSON_SEPARATOR"; }
