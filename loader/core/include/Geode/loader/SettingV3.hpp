#pragma once

#include "../core/Prelude.hpp"
#include <optional>
#include <concepts>
// #include <cocos2d.h>
// #include "../utils/cocos.hpp"
#include "../utils/file.hpp"
// this unfortunately has to be included because of C++ templates
#include "../utils/JsonValidation.hpp"
#include "../utils/function.hpp"

class ModSettingsPopup;

namespace geode {
    class ModSettingsManager;
    class SettingNodeV3;
    class TitleSettingV3;
    class BoolSettingV3;
    class IntSettingV3;
    class FloatSettingV3;
    class StringSettingV3;
    class FileSettingV3;
    class Color3BSettingV3;
    class Color4BSettingV3;

    class SettingNodeV3Visitor {
    public:
        virtual ~SettingNodeV3Visitor() = default;
        virtual SettingNodeV3* visit(TitleSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(BoolSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(IntSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(FloatSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(StringSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(FileSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(Color3BSettingV3* sett, float width) = 0;
        virtual SettingNodeV3* visit(Color4BSettingV3* sett, float width) = 0;
    };

    class GEODE_DLL SettingV3 : public std::enable_shared_from_this<SettingV3> {
    private:
        class GeodeImpl;
        std::shared_ptr<GeodeImpl> m_impl;
    
    protected:
        /**
         * Only call this function if you aren't going to call 
         * `parseBaseProperties`, which will call it for you!
         * If you don't want to call `parseBaseProperties`, at the very least 
         * you **must** call this!
         * Select which properties you want to parse using the `parseX` 
         * functions
         * @param key The setting's key as defined in `mod.json`
         * @param modID The ID of the mod this settings is being parsed for 
         * @param json The current JSON checking instance being used. This 
         * should be the JSON object that defines the setting. If you aren't 
         * using Geode's JSON checking utilities, you can use the other 
         * overload of `init`
         */
        void init(std::string const& key, std::string const& modID, JsonExpectedValue& json);
        /**
         * Only call this function if you aren't going to call 
         * `parseBaseProperties`, which will call it for you!
         * If you don't want to call `parseBaseProperties`, at the very least 
         * you **must** call this!
         * Select which properties you want to parse using the `parseX` 
         * functions
         * @param key The setting's key as defined in `mod.json`
         * @param modID The ID of the mod this settings is being parsed for 
         * @note If you are using Geode's JSON checking utilities 
         * (`checkJson` / `JsonExpectedValue`), you should be using the other 
         * overload that takes a `JsonExpectedValue&`!
         */
        void init(std::string const& key, std::string const& modID);

        /**
         * Parses the `"name"` and `"description"` keys from the setting's 
         * definition in `mod.json` (if they exist), so their values can be 
         * accessed via `getName` and `getDescription`.
         * @param json The current JSON checking instance being used. This 
         * should be the JSON object that defines the setting
         * @warning In most cases, you should be using `parseBaseProperties` 
         * instead to do all of this in one go! 
         * If you do need the fine-grained control however, make sure to call 
         * `init` before calling these parsing functions!
         */
        void parseNameAndDescription(JsonExpectedValue& json);
        /**
         * Parses the `"enable-if"` and `"enable-if-description"` keys from 
         * the setting's definition in `mod.json` (if they exist), so 
         * `shouldEnable` and `getEnableIfDescription` work.
         * @param json The current JSON checking instance being used. This 
         * should be the JSON object that defines the setting
         * @warning In most cases, you should be using `parseBaseProperties` 
         * instead to do all of this in one go! 
         * If you do need the fine-grained control however, make sure to call 
         * `init` before calling these parsing functions!
         */
        void parseEnableIf(JsonExpectedValue& json);
        /**
         * Parses the `"requires-restart"` key from the setting's definition in 
         * `mod.json` (if they exist), so `requiresRestart` works.
         * @param json The current JSON checking instance being used. This 
         * should be the JSON object that defines the setting
         * @warning In most cases, you should be using `parseBaseProperties` 
         * instead to do all of this in one go! 
         * If you do need the fine-grained control however, make sure to call 
         * `init` before calling these parsing functions!
         */
        void parseValueProperties(JsonExpectedValue& json);

        /**
         * Parse all of the base properties such as `"name"` and `"description"` 
         * for this setting
         * @param key The setting's key as defined in `mod.json`
         * @param modID The ID of the mod this settings is being parsed for 
         * @param json The current JSON checking instance being used. If you 
         * aren't using Geode's JSON checking utilities, use the other overload 
         * of this function
         * @note If you don't want to parse some of the base properties, such as 
         * `"requires-restart"` (because you're doing a cosmetic setting), then 
         * you can call `init` instead and then the specific `parseX` functions
         */
        void parseBaseProperties(std::string const& key, std::string const& modID, JsonExpectedValue& json);
        /**
         * Parse all of the base properties such as `"name"` and `"description"` 
         * for this setting
         * @param key The setting's key as defined in `mod.json`
         * @param modID The ID of the mod this settings is being parsed for 
         * @param json The JSON value. If you are using Geode's JSON checking 
         * utilities (`checkJson` / `JsonExpectedValue`), you should use the 
         * other overload directly!
         * @note If you don't want to parse some of the base properties, such as 
         * `"requires-restart"` (because you're doing a cosmetic setting), then 
         * you can call `init` instead and then the specific `parseX` functions
         */
        Result<> parseBaseProperties(std::string const& key, std::string const& modID, matjson::Value const& json);

        /**
         * Mark that the value of this setting has changed. This should be 
         * ALWAYS called on every setter that can modify the setting's state!
         */
        void markChanged();

    public:
        SettingV3();
        virtual ~SettingV3();

        /**
         * Get the key of this setting
         */
        std::string getKey() const;
        /**
         * Get the mod ID this setting is for
         */
        std::string getModID() const;
        /**
         * Get the mod this setting is for. Note that this may return null 
         * while the mod is still being initialized
         */
        Mod* getMod() const;
        /**
         * Get the name of this setting
         */
        std::optional<std::string> getName() const; 
        /**
         * Get the name of this setting, or its key if it has no name
         */
        std::string getDisplayName() const; 
        /**
         * Get the description of this setting
         */
        std::optional<std::string> getDescription() const;
        /**
         * Get the "enable-if" scheme for this setting
         */
        std::optional<std::string> getEnableIf() const;
        /**
         * Check if this setting should be enabled based on the "enable-if" scheme
         */
        bool shouldEnable() const;
        std::optional<std::string> getEnableIfDescription() const;
        /**
         * Whether this setting requires a restart on change
         */
        bool requiresRestart() const;
        /**
         * Get the platforms this setting is available on
         */
        std::vector<PlatformID> getPlatforms() const;

        virtual bool load(matjson::Value const& json) = 0;
        virtual bool save(matjson::Value& json) const = 0;
        virtual SettingNodeV3* createNode(float width);
        virtual SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor);

        virtual bool isDefaultValue() const = 0;
        /**
         * Reset this setting's value back to its original value
         */
        virtual void reset() = 0;
    };
    
    using SettingGeneratorV3 = std::function<Result<std::shared_ptr<SettingV3>>(
        std::string const& key,
        std::string const& modID,
        matjson::Value const& json
    )>;

    /**
     * A helper class for creating a basic setting with a simple value. 
     * Override the virtual function `isValid` to 
     * @tparam T The type of the setting's value. This type must be JSON-
     * serializable and deserializable!
     * @tparam V The type used for the `setValue` function, if it differs from T
     */
    template <class T, class V = T>
    class SettingBaseValueV3 : public SettingV3 {
    private:
        class Impl final {
        private:
            T defaultValue;
            T value;
            friend class SettingBaseValueV3;
        };
        std::shared_ptr<Impl> m_impl;
    
    protected:
        /**
         * Parses the `"default"` key from the setting's definition in 
         * `mod.json`. The key may also be defined per-platform, i.e. 
         * `"default": { "win": ..., "android": ... }`
         * @param json The current JSON checking instance being used. This 
         * should be the JSON object that defines the setting
         * @warning In most cases, you should be using `parseBaseProperties` 
         * instead to do all of this in one go! 
         * If you do need the fine-grained control however, make sure to call 
         * `init` before calling these parsing functions!
         */
        void parseDefaultValue(JsonExpectedValue& json) {
            auto root = json.needs("default");
            // Check if this is a platform-specific default value
            if (root.isObject() && root.has(GEODE_PLATFORM_SHORT_IDENTIFIER_NOARCH)) {
                root.needs(GEODE_PLATFORM_SHORT_IDENTIFIER_NOARCH).into(m_impl->defaultValue);
            }
            else {
                root.into(m_impl->defaultValue);
            }
            m_impl->value = m_impl->defaultValue;
        }

        /**
         * Parse shared value, including the default value for this setting
         * @param key The key of the setting
         * @param modID The ID of the mod this setting is being parsed for
         * @param json The current JSON checking instance being used. If you 
         * aren't using Geode's JSON checking utilities, use the other overload 
         * of this function
         */
        void parseBaseProperties(std::string const& key, std::string const& modID, JsonExpectedValue& json) {
            SettingV3::parseBaseProperties(key, modID, json);
            this->parseDefaultValue(json);
        }
        /**
         * Parse shared value, including the default value for this setting
         * @param key The key of the setting
         * @param modID The ID of the mod this setting is being parsed for
         * @param json The JSON value. If you are using Geode's JSON checking 
         * utilities (`checkJson` / `JsonExpectedValue`), you should use the 
         * other overload directly!
         */
        Result<> parseBaseProperties(std::string const& key, std::string const& modID, matjson::Value const& json) {
            auto root = checkJson(json, "SettingBaseValueV3");
            this->parseBaseProperties(key, modID, root);
            return root.ok();
        }

        /**
         * Set the default value. This does not check that the value is 
         * actually valid!
         */
        void setDefaultValue(V value) {
            m_impl->defaultValue = value;
        }

    public:
        SettingBaseValueV3() : m_impl(std::make_shared<Impl>()) {}

        using ValueType = T;
        using ValueAssignType = V;

        /**
         * Get the default value for this setting
         */
        T getDefaultValue() const {
            return m_impl->defaultValue;
        }

        /**
         * Get the current value of this setting
         */
        T getValue() const {
            return m_impl->value;
        }
        /**
         * Set the value of this setting. This will broadcast a new 
         * SettingChangedEventV3, letting any listeners now the value has changed
         * @param value The new value for the setting. If the value is not a 
         * valid value for this setting (as determined by `isValue`), then the 
         * setting's value is reset to the default value
         */
        void setValue(V value) {
            m_impl->value = this->isValid(value) ? value : m_impl->defaultValue;
            this->markChanged();
        }
        /**
         * Check if a given value is valid for this setting. If not, an error 
         * describing why the value isn't valid is returned
         */
        virtual Result<> isValid(V value) const {
            return Ok();
        }
        
        bool isDefaultValue() const override {
            return m_impl->value == m_impl->defaultValue;
        }
        void reset() override {
            this->setValue(m_impl->defaultValue);
        }

        bool load(matjson::Value const& json) override {
            auto res = json.as<T>();
            if (res.isErr()) {
                return false;
            }
            m_impl->value = res.unwrap();
            return true;
        }
        bool save(matjson::Value& json) const override {
            json = matjson::Value(m_impl->value);
            return true;
        }
    };

    class GEODE_DLL TitleSettingV3 final : public SettingV3 {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;
    
    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        TitleSettingV3(PrivateMarker);
        static Result<std::shared_ptr<TitleSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        bool load(matjson::Value const& json) override;
        bool save(matjson::Value& json) const override;
        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;

        bool isDefaultValue() const override;
        void reset() override;
    };

    class GEODE_DLL BoolSettingV3 final : public SettingBaseValueV3<bool> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        BoolSettingV3(PrivateMarker);
        static Result<std::shared_ptr<BoolSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(bool value) const override;
        
        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL IntSettingV3 final : public SettingBaseValueV3<int64_t> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        IntSettingV3(PrivateMarker);
        static Result<std::shared_ptr<IntSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(int64_t value) const override;

        std::optional<int64_t> getMinValue() const;
        std::optional<int64_t> getMaxValue() const;

        bool isArrowsEnabled() const;
        bool isBigArrowsEnabled() const;
        size_t getArrowStepSize() const;
        size_t getBigArrowStepSize() const;
        bool isSliderEnabled() const;
        int64_t getSliderSnap() const;
        bool isInputEnabled() const;
    
        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL FloatSettingV3 final : public SettingBaseValueV3<double> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        FloatSettingV3(PrivateMarker);
        static Result<std::shared_ptr<FloatSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(double value) const override;

        std::optional<double> getMinValue() const;
        std::optional<double> getMaxValue() const;

        bool isArrowsEnabled() const;
        bool isBigArrowsEnabled() const;
        double getArrowStepSize() const;
        double getBigArrowStepSize() const;
        bool isSliderEnabled() const;
        double getSliderSnap() const;
        bool isInputEnabled() const;
        
        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL StringSettingV3 final : public SettingBaseValueV3<std::string, std::string_view> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        StringSettingV3(PrivateMarker);
        static Result<std::shared_ptr<StringSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(std::string_view value) const override;

        std::optional<std::string> getRegexValidator() const;
        std::optional<std::string> getAllowedCharacters() const;
        std::optional<std::vector<std::string>> getEnumOptions() const;
        
        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL FileSettingV3 final : public SettingBaseValueV3<std::filesystem::path, std::filesystem::path const&> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        FileSettingV3(PrivateMarker);
        static Result<std::shared_ptr<FileSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(std::filesystem::path const& value) const override;

        bool isFolder() const;
        bool useSaveDialog() const;

        std::optional<std::vector<utils::file::FilePickOptions::Filter>> getFilters() const;
        
        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL Color3BSettingV3 final : public SettingBaseValueV3<Color3B> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        Color3BSettingV3(PrivateMarker);
        static Result<std::shared_ptr<Color3BSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(Color3B value) const override;

        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL Color4BSettingV3 final : public SettingBaseValueV3<Color4B> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;

    private:
        class PrivateMarker {};
        friend class SettingV3;

    public:
        Color4BSettingV3(PrivateMarker);
        static Result<std::shared_ptr<Color4BSettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

        Result<> isValid(Color4B value) const override;

        SettingNodeV3* createNode(float width, SettingNodeV3Visitor* visitor) override;
    };

    class GEODE_DLL SettingChangedEventV3 final : public Event {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;
    
    public:
        SettingChangedEventV3(std::shared_ptr<SettingV3> setting);

        std::shared_ptr<SettingV3> getSetting() const;
    };
    class GEODE_DLL SettingChangedFilterV3 final : public EventFilter<SettingChangedEventV3> {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;
    
    public:
        using Callback = void(std::shared_ptr<SettingV3>);

        ListenerResult handle(std::function<Callback> fn, SettingChangedEventV3* event);
        /**
         * Listen to changes on a setting, or all settings
         * @param modID Mod whose settings to listen to
         * @param settingKey Setting to listen to, or all settings if nullopt
         */
        SettingChangedFilterV3(
            std::string const& modID,
            std::optional<std::string> const& settingKey
        );
        SettingChangedFilterV3(Mod* mod, std::optional<std::string> const& settingKey);
        SettingChangedFilterV3(SettingChangedFilterV3 const&);
    };

    template <class T>
    struct SettingTypeForValueType {
        static_assert(
            !std::is_same_v<T, T>,
            "specialize the SettingTypeForValueType class to use Mod::getSettingValue for custom settings"
        );
    };

    template <>
    struct SettingTypeForValueType<bool> {
        using SettingType = BoolSettingV3;
    };
    template <std::integral T>
    struct SettingTypeForValueType<T> {
        using SettingType = IntSettingV3;
    };
    template <std::floating_point T>
    struct SettingTypeForValueType<T> {
        using SettingType = FloatSettingV3;
    };
    template <>
    struct SettingTypeForValueType<std::string> {
        using SettingType = StringSettingV3;
    };
    template <>
    struct SettingTypeForValueType<std::filesystem::path> {
        using SettingType = FileSettingV3;
    };
    template <>
    struct SettingTypeForValueType<Color3B> {
        using SettingType = Color3BSettingV3;
    };
    template <>
    struct SettingTypeForValueType<Color4B> {
        using SettingType = Color4BSettingV3;
    };

    template <class T>
    EventListener<SettingChangedFilterV3>* listenForSettingChangesV3(std::string_view settingKey, auto&& callback, Mod* mod = getMod()) {
        using Ty = typename SettingTypeForValueType<T>::SettingType;
        return new EventListener(
            [callback = std::move(callback)](std::shared_ptr<SettingV3> setting) {
                if (auto ty = geode::cast::typeinfo_pointer_cast<Ty>(setting)) {
                    callback(ty->getValue());
                }
            },
            SettingChangedFilterV3(mod, std::string(settingKey))
        );
    }
    EventListener<SettingChangedFilterV3>* listenForSettingChangesV3(std::string_view settingKey, auto&& callback, Mod* mod = getMod()) {
        using T = std::remove_cvref_t<utils::function::Arg<0, decltype(callback)>>;
        return listenForSettingChangesV3<T>(settingKey, std::move(callback), mod);
    }
    GEODE_DLL EventListener<SettingChangedFilterV3>* listenForAllSettingChangesV3(
        std::function<void(std::shared_ptr<SettingV3>)> const& callback,
        Mod* mod = getMod()
    );
}
