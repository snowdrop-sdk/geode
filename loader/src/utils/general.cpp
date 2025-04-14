#include <Geode/utils/general.hpp>

using namespace geode::prelude;

#ifndef GEODE_IS_MACOS
// feel free to properly implement this for other platforms
float geode::utils::getDisplayFactor() {
    return 1.0f;
}
#endif

Result<Color3B, std::string> matjson::Serialize<Color3B>::fromJson(matjson::Value const& value) {
    if (value.isArray()) {
        auto arr = GEODE_UNWRAP(value.asArray());
        if (arr.size() == 3) {
            auto r = GEODE_UNWRAP(arr[0].as<uint8_t>());
            auto g = GEODE_UNWRAP(arr[1].as<uint8_t>());
            auto b = GEODE_UNWRAP(arr[2].as<uint8_t>());
            return Ok(Color3B{r, g, b});
        }
        return Err("Expected color array to have 3 items");
    }
    if (value.isObject()) {
        auto r = GEODE_UNWRAP(GEODE_UNWRAP(value.get("r")).as<uint8_t>());
        auto g = GEODE_UNWRAP(GEODE_UNWRAP(value.get("g")).as<uint8_t>());
        auto b = GEODE_UNWRAP(GEODE_UNWRAP(value.get("b")).as<uint8_t>());
        return Ok(Color3B{r, g, b});
    }
    if (value.isString()) {
        auto hex = GEODE_UNWRAP(value.asString());
        auto res = cc3bFromHexString(hex);
        if (!res) {
            return Err("Invalid hex color string: {}", res.unwrapErr());
        }
        return Ok(res.unwrap());
    }
    return Err("Expected color to be array, object or hex string");
}
matjson::Value matjson::Serialize<Color3B>::toJson(Color3B const& value) {
    return matjson::makeObject({
        { "r", value.r },
        { "g", value.g },
        { "b", value.b }
    });
}

Result<Color4B, std::string> matjson::Serialize<Color4B>::fromJson(matjson::Value const& value) {
    if (value.isArray()) {
        auto arr = GEODE_UNWRAP(value.asArray());
        if (arr.size() == 4) {
            auto r = GEODE_UNWRAP(arr[0].as<uint8_t>());
            auto g = GEODE_UNWRAP(arr[1].as<uint8_t>());
            auto b = GEODE_UNWRAP(arr[2].as<uint8_t>());
            auto a = GEODE_UNWRAP(arr[3].as<uint8_t>());
            return Ok(Color4B{r, g, b, a});
        }
        return Err("Expected color array to have 4 items");
    }
    if (value.isObject()) {
        auto r = GEODE_UNWRAP(GEODE_UNWRAP(value.get("r")).as<uint8_t>());
        auto g = GEODE_UNWRAP(GEODE_UNWRAP(value.get("g")).as<uint8_t>());
        auto b = GEODE_UNWRAP(GEODE_UNWRAP(value.get("b")).as<uint8_t>());
        auto a = GEODE_UNWRAP(GEODE_UNWRAP(value.get("a")).as<uint8_t>());
        return Ok(Color4B{r, g, b, a});
    }
    if (value.isString()) {
        auto hex = GEODE_UNWRAP(value.asString());
        auto res = cc4bFromHexString(hex);
        if (!res) {
            return Err("Invalid hex color string: {}", res.unwrapErr());
        }
        return Ok(res.unwrap());
    }
    return Err("Expected color to be array, object or hex string");
}

matjson::Value matjson::Serialize<Color4B>::toJson(Color4B const& value) {
    return matjson::makeObject({
        { "r", value.r },
        { "g", value.g },
        { "b", value.b },
        { "a", value.a }
    });
}

Result<Color3B> geode::cc3bFromHexString(std::string const& rawHexValue, bool permissive) {
    if (permissive && rawHexValue.empty()) {
        return Ok(Color3B{255, 255, 255});
    }
    auto hexValue = rawHexValue;
    if (hexValue[0] == '#') {
        hexValue.erase(hexValue.begin());
    }
    if (hexValue.size() > 6) {
        return Err("Hex value too large");
    }
    auto res = numFromString<uint32_t>(hexValue, 16);
    if (!res) {
        return Err("Invalid hex value '{}'", hexValue);
    }
    auto numValue = res.unwrap();
    switch (hexValue.size()) {
        case 6: {
            auto r = static_cast<uint8_t>((numValue & 0xFF0000) >> 16);
            auto g = static_cast<uint8_t>((numValue & 0x00FF00) >> 8);
            auto b = static_cast<uint8_t>((numValue & 0x0000FF));
            return Ok(Color3B{r, g, b});
        } break;

        case 3: {
            auto r = static_cast<uint8_t>(((numValue & 0xF00) >> 8) * 17);
            auto g = static_cast<uint8_t>(((numValue & 0x0F0) >> 4) * 17);
            auto b = static_cast<uint8_t>(((numValue & 0x00F)) * 17);
            return Ok(Color3B{r, g, b});
        } break;

        case 2: {
            if (!permissive) {
                return Err("Invalid hex pattern, expected RGB or RRGGBB");
            }
            auto num = static_cast<uint8_t>(numValue);
            return Ok(Color3B{num, num, num});
        } break;

        case 1: {
            if (!permissive) {
                return Err("Invalid hex pattern, expected RGB or RRGGBB");
            }
            auto num = static_cast<uint8_t>(numValue * 17);
            return Ok(Color3B{num, num, num});
        } break;

        default: {
            if (permissive) {
                return Err("Invalid hex pattern, expected R, RR, RGB, or RRGGBB");
            }
            else {
                return Err("Invalid hex pattern, expected RGB or RRGGBB");
            }
        }
    }
}

Result<Color4B> geode::cc4bFromHexString(std::string const& rawHexValue, bool requireAlpha, bool permissive) {
    if (permissive && rawHexValue.empty()) {
        return Ok(Color4B{255, 255, 255, 255});
    }
    auto hexValue = rawHexValue;
    if (hexValue[0] == '#') {
        hexValue.erase(hexValue.begin());
    }
    if (hexValue.size() > 8) {
        return Err("Hex value too large");
    }
    auto res = numFromString<uint32_t>(hexValue, 16);
    if (!res) {
        return Err("Invalid hex value '{}'", hexValue);
    }
    auto numValue = res.unwrap();
    switch (hexValue.size()) {
        case 8: {
            auto r = static_cast<uint8_t>((numValue & 0xFF000000) >> 24);
            auto g = static_cast<uint8_t>((numValue & 0x00FF0000) >> 16);
            auto b = static_cast<uint8_t>((numValue & 0x0000FF00) >> 8);
            auto a = static_cast<uint8_t>((numValue & 0x000000FF));
            return Ok(Color4B{r, g, b, a});
        } break;

        case 6: {
            if (requireAlpha) {
                return Err("Alpha component is required, got only RRGGBB");
            }
            auto r = static_cast<uint8_t>((numValue & 0xFF0000) >> 16);
            auto g = static_cast<uint8_t>((numValue & 0x00FF00) >> 8);
            auto b = static_cast<uint8_t>((numValue & 0x0000FF));
            return Ok(Color4B{r, g, b, 255});
        } break;

        case 4: {
            auto r = static_cast<uint8_t>(((numValue & 0xF000) >> 12) * 17);
            auto g = static_cast<uint8_t>(((numValue & 0x0F00) >> 8) * 17);
            auto b = static_cast<uint8_t>(((numValue & 0x00F0) >> 4) * 17);
            auto a = static_cast<uint8_t>(((numValue & 0x000F)) * 17);
            return Ok(Color4B{r, g, b, a});
        } break;

        case 3: {
            if (requireAlpha) {
                return Err("Alpha component is required, got only RGB");
            }
            auto r = static_cast<uint8_t>(((numValue & 0xF00) >> 8) * 17);
            auto g = static_cast<uint8_t>(((numValue & 0x0F0) >> 4) * 17);
            auto b = static_cast<uint8_t>(((numValue & 0x00F)) * 17);
            return Ok(Color4B{r, g, b, 255});
        } break;

        case 2: {
            if (!permissive) {
                return Err("Invalid hex pattern, expected RGBA or RRGGBBAA");
            }
            if (requireAlpha) {
                return Err("Alpha component is required, specify full RRGGBBAA");
            }
            auto num = static_cast<uint8_t>(numValue);
            return Ok(Color4B{num, num, num, 255});
        } break;

        case 1: {
            if (!permissive) {
                return Err("Invalid hex pattern, expected RGBA or RRGGBBAA");
            }
            if (requireAlpha) {
                return Err("Alpha component is required, specify full RGBA");
            }
            auto num = static_cast<uint8_t>(numValue * 17);
            return Ok(Color4B{num, num, num, 255});
        } break;

        default: {
            if (requireAlpha) {
                return Err("Invalid hex pattern, expected RGBA or RRGGBBAA");
            }
            else if (permissive) {
                return Err("Invalid hex pattern, expected R, RR, RGB, RGBA, RRGGBB, or RRGGBBAA");
            }
            else {
                return Err("Invalid hex pattern, expected RGB, RGBA, RRGGBB, or RRGGBBAA");
            }
        }
    }
}

std::string geode::cc3bToHexString(Color3B const& color) {
    static constexpr auto digits = "0123456789ABCDEF";
    std::string output;
    output += digits[color.r >> 4 & 0xF];
    output += digits[color.r & 0xF];
    output += digits[color.g >> 4 & 0xF];
    output += digits[color.g & 0xF];
    output += digits[color.b >> 4 & 0xF];
    output += digits[color.b & 0xF];
    return output;
}

std::string geode::cc4bToHexString(Color4B const& color) {
    static constexpr auto digits = "0123456789ABCDEF";
    std::string output;
    output += digits[color.r >> 4 & 0xF];
    output += digits[color.r & 0xF];
    output += digits[color.g >> 4 & 0xF];
    output += digits[color.g & 0xF];
    output += digits[color.b >> 4 & 0xF];
    output += digits[color.b & 0xF];
    output += digits[color.a >> 4 & 0xF];
    output += digits[color.a & 0xF];
    return output;
}
