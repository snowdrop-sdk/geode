#pragma once

#include "../DefaultInclude.hpp"
#include <cocos2d.h>
#include "../../loader/SettingV3.hpp"

namespace geode {

    class GEODE_DLL SettingNodeV3Visitor {
    public:
        virtual ~SettingNodeV3Visitor() = default;
        virtual void visit(SettingV3* setting) = 0;
    };
    
    class GEODE_DLL SettingNodeV3 : public cocos2d::CCNode, public SettingNodeV3Visitor {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;
        
        friend class ::ModSettingsPopup;

    protected:
        bool init(std::shared_ptr<SettingV3> setting, float width);

        /**
         * Update the state of this setting node, bringing all inputs 
         * up-to-date with the current value. Derivatives of `SettingNodeV3` 
         * should set update the state (such as visibility, value, etc.) of all 
         * its controls, except for the one that's passed as the `invoker` 
         * argument. Derivatives should remember to **always call the base 
         * class's `updateState` function**, as it updates the built-in title 
         * label as well as the description and reset buttons!
         * @param invoker The button or other interactive element that caused 
         * this state update. If that element is for example a text input, it 
         * may wish to ignore the state update, as it itself is the source of 
         * truth for the node's value at that moment. May be nullptr to mark 
         * that no specific node requested this state update
         */
        virtual void updateState(cocos2d::CCNode* invoker);

        /**
         * Mark this setting as changed. This updates the UI for committing 
         * the value, as well as posts a `SettingNodeValueChangeEventV3`
         * @param invoker The node to be passed onto `updateState`
         */
        void markChanged(cocos2d::CCNode* invoker);

        /**
         * When the setting value is committed (aka can't be undone), this 
         * function will be called. This should take care of actually saving 
         * the value in some sort of global manager
         */
        virtual void onCommit() = 0;
        virtual void onResetToDefault() = 0;

        void onDescription(CCObject*);
        void onReset(CCObject*);

    public:
        void commit();
        void resetToDefault();
        virtual bool hasUncommittedChanges() const = 0;
        virtual bool hasNonDefaultValue() const = 0;

        virtual void visit(SettingV3* setting) = 0;

        // Can be overridden by the setting itself
        // Can / should be used to do alternating BG
        void setDefaultBGColor(Color4B color);

        cocos2d::CCLabelBMFont* getNameLabel() const;
        cocos2d::CCLabelBMFont* getStatusLabel() const;
        cocos2d::CCMenu* getNameMenu() const;
        cocos2d::CCMenu* getButtonMenu() const;
        cocos2d::CCLayerColor* getBG() const;

        void setContentSize(cocos2d::CCSize const& size) override;

        std::shared_ptr<SettingV3> getSetting() const;
    };

    /**
     * Helper class for creating `SettingNode`s for simple settings that 
     * implement `SettingBaseValueV3`
     */
    template <class S>
    class SettingValueNodeV3 : public SettingNodeV3 {
    protected:
    private:
        class Impl final {
        private:
            typename S::ValueType currentValue;
            friend class SettingValueNodeV3;
        };
        std::shared_ptr<Impl> m_impl;
    
    protected:
        bool init(std::shared_ptr<S> setting, float width) {
            if (!SettingNodeV3::init(setting, width))
                return false;
            
            m_impl = std::make_shared<Impl>();
            m_impl->currentValue = setting->getValue();

            return true;
        }

        void updateState(cocos2d::CCNode* invoker) {
            SettingNodeV3::updateState(invoker);
            auto validate = this->getSetting()->isValid(m_impl->currentValue);
            if (!validate) {
                this->getStatusLabel()->setVisible(true);
                this->getStatusLabel()->setString(validate.unwrapErr().c_str());
                this->getStatusLabel()->setColor(cocos2d::ccc3(235, 35, 52));
            }
        }

        void onCommit() override {
            this->getSetting()->setValue(m_impl->currentValue);
            // The value may be different, if the current value was an invalid
            // value for the setting
            this->setValue(this->getSetting()->getValue(), nullptr);
        }
        bool hasUncommittedChanges() const override {
            return m_impl->currentValue != this->getSetting()->getValue();
        }
        bool hasNonDefaultValue() const override {
            return m_impl->currentValue != this->getSetting()->getDefaultValue();
        }
        void onResetToDefault() override {
            this->setValue(this->getSetting()->getDefaultValue(), nullptr);
        }

    public:
        /**
         * Get the **uncommitted** value for this node
         */
        typename S::ValueType getValue() const {
            return m_impl->currentValue;
        }
        /**
         * Set the **uncommitted** value for this node
         * @param value The value to set
         * @param invoker The node that invoked this value change; see the docs 
         * for `SettingNodeV3::updateState` to know more
         */
        void setValue(typename S::ValueAssignType value, cocos2d::CCNode* invoker) {
            m_impl->currentValue = value;
            this->markChanged(invoker);
        }

        std::shared_ptr<S> getSetting() const {
            return std::static_pointer_cast<S>(SettingNodeV3::getSetting());
        }
    };

    class GEODE_DLL SettingNodeSizeChangeEventV3 : public Event {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;
    
    public:
        SettingNodeSizeChangeEventV3(SettingNodeV3* node);
        virtual ~SettingNodeSizeChangeEventV3();

        SettingNodeV3* getNode() const;
    };
    class GEODE_DLL SettingNodeValueChangeEventV3 : public Event {
    private:
        class Impl;
        std::shared_ptr<Impl> m_impl;
    
    public:
        SettingNodeValueChangeEventV3(SettingNodeV3* node, bool commit);
        virtual ~SettingNodeValueChangeEventV3();

        SettingNodeV3* getNode() const;
        bool isCommit() const;
    };

    using SettingNode = SettingNodeV3;
    template <class S>
    using SettingValueNode = SettingValueNodeV3<S>;
    using SettingNodeSizeChangeEvent = SettingNodeSizeChangeEventV3;
    using SettingNodeValueChangeEvent = SettingNodeValueChangeEventV3;
}