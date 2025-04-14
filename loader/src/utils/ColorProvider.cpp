#include <Geode/utils/ColorProvider.hpp>

using namespace geode::prelude;

static inline Color4B to4B(Color3B const& color) {
    return { color.r, color.g, color.b, 255 };
}

static inline Color3B to3B(Color4B const& color) {
    return { color.r, color.g, color.b };
}

ColorProvidedEvent::ColorProvidedEvent(std::string const& id, Color4B const& color)
  : id(id), color(color) {}

ListenerResult ColorProvidedFilter::handle(std::function<Callback> fn, ColorProvidedEvent* event) {
    if (event->id == m_id) {
        fn(event);
    }
    return ListenerResult::Propagate;
}

ColorProvidedFilter::ColorProvidedFilter(std::string const& id) : m_id(id) {}

class ColorProvider::Impl {
public:
    std::unordered_map<std::string, std::pair<Color4B, std::optional<Color4B>>> colors;
};

ColorProvider::ColorProvider() : m_impl(new Impl()) {}

ColorProvider* ColorProvider::get() {
    static auto inst = new ColorProvider();
    return inst;
}

Color4B ColorProvider::define(std::string const& id, Color4B const& color) {
    // `insert` doesn't override existing keys, which is what we want
    m_impl->colors.insert({ id, std::pair(color, std::nullopt) });
    return this->color(id);
}
Color3B ColorProvider::define(std::string const& id, Color3B const& color) {
    return to3B(this->define(id, to4B(color)));
}
Color4B ColorProvider::override(std::string const& id, Color4B const& color) {
    if (m_impl->colors.contains(id)) {
        m_impl->colors.at(id).second = color;
        ColorProvidedEvent(id, color).post();
        return color;
    }
    else {
        log::error("(ColorProvider) Attempted to override color \"{}\", which is not defined", id);
        return {255, 255, 255, 255};
    }
}
Color3B ColorProvider::override(std::string const& id, Color3B const& color) {
    return to3B(this->override(id, to4B(color)));
}
Color4B ColorProvider::reset(std::string const& id) {
    if (m_impl->colors.contains(id)) {
        m_impl->colors.at(id).second = std::nullopt;
        auto def = m_impl->colors.at(id).first;
        ColorProvidedEvent(id, def).post();
        return def;
    }
    else {
        log::error("(ColorProvider) Attempted to reset color \"{}\", which is not defined", id);
        return {255, 255, 255, 255};
    }
}
Color4B ColorProvider::color(std::string const& id) const {
    if (m_impl->colors.contains(id)) {
        return m_impl->colors.at(id).second.value_or(m_impl->colors.at(id).first);
    }
    else {
        log::error("(ColorProvider) Attempted to get color \"{}\", which is not defined", id);
        return {255, 255, 255, 255};
    }
}
Color3B ColorProvider::color3b(std::string const& id) const {
    return to3B(this->color(id));
}
