#include "../core/Prelude.hpp"
#include <string_view>

namespace geode::bridge {
    GEODE_DLL void createGamePopup(
        std::string_view title,
        std::string_view text,
        std::string_view confirmText,
        std::string_view cancelText,
        std::function<void(bool)> callback
    );
}