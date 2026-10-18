
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class ALCompletePopup : public FLAlertLayer {
protected:
    bool init() {
        if (!FLAlertLayer::init(
            nullptr,
            "ALComplete",
            "Welcome to ALComplete!",
            "Close",
            nullptr,
            300.f,
            false,
            150.f,
            1.f
        )) {
            return false;
        }

        return true;
    }

public:
    static ALCompletePopup* create() {
        auto ret = new ALCompletePopup();

        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

class $modify(ALCompleteMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto icon = CCSprite::createWithSpriteFrameName(
            "GJ_optionsBtn_001.png"
        );

        if (!icon) {
            icon = CCSprite::create();
        }

        auto buttonSprite = CircleButtonSprite::create(
            icon,
            CircleBaseColor::Green,
            CircleBaseSize::Medium
        );

        if (!buttonSprite) {
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(ALCompleteMenuLayer::onALComplete)
        );

        if (button) {
            button->setID("alcomplete-button");
            button->setPosition(
                ccp(winSize.width - 35.f, winSize.height - 75.f)
            );

            auto menu = CCMenu::create();
            menu->setID("alcomplete-menu");
            menu->setPosition(CCPointZero);
            menu->addChild(button);
            this->addChild(menu);
        }

        return true;
    }

    void onALComplete(CCObject*) {
        auto popup = ALCompletePopup::create();

        if (popup) {
            popup->show();
        }
    }
};
