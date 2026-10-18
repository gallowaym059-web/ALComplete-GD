#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <cstdint>

using namespace geode::prelude;

namespace {
    class ALCompletePopup final : public FLAlertLayer {
    public:
        static void open() {
            auto popup = new ALCompletePopup();
            if (popup->init(240.f, 170.f, "GJ_square01.png", "ALComplete")) {
                popup->autorelease();
                popup->show();
            } else {
                delete popup;
            }
        }

        bool init(float width, float height, char const* bg, char const* title) {
            if (!FLAlertLayer::init(width, height, bg, title, "Close", nullptr, 300.f)) {
                return false;
            }

            // Use the popup's local content coordinates, not the full screen center.
            auto const contentSize = m_mainLayer->getContentSize();
            auto const center = ccp(contentSize.width / 2.f, contentSize.height / 2.f);

            auto heading = CCLabelBMFont::create("QUICK ACTIONS", "bigFont.fnt");
            if (heading) {
                heading->setScale(0.42f);
                heading->setPosition(ccp(center.x, center.y + 33.f));
                m_mainLayer->addChild(heading);
            }

            auto info = CCLabelBMFont::create("Complete current level", "goldFont.fnt");
            if (info) {
                info->setScale(0.48f);
                info->setPosition(ccp(center.x, center.y + 6.f));
                m_mainLayer->addChild(info);
            }

            auto completeLabel = CCLabelBMFont::create("COMPLETE", "bigFont.fnt");
            auto menu = CCMenu::create();
            if (completeLabel && menu) {
                completeLabel->setScale(0.45f);
                auto completeButton = CCMenuItemSpriteExtra::create(
                    completeLabel, this, menu_selector(ALCompletePopup::onComplete)
                );
                if (completeButton) {
                    menu->addChild(completeButton);
                    menu->setPosition(ccp(center.x, center.y - 25.f));
                    m_mainLayer->addChild(menu);
                }
            }

            auto note = CCLabelBMFont::create("Settings: Geode > ALComplete", "chatFont.fnt");
            if (note) {
                note->setScale(0.42f);
                note->setOpacity(180);
                note->setPosition(ccp(center.x, center.y - 52.f));
                m_mainLayer->addChild(note);
            }
            return true;
        }

        void onComplete(CCObject*) {
            auto layer = PlayLayer::get();
            if (!layer) {
                FLAlertLayer::create("ALComplete", "Open a level first.", "OK")->show();
                return;
            }
            // Calls the game's completion routine directly; this is not bot playback.
            layer->levelComplete();
        }
    };
}

class $modify(ALCompletePlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }
        if (!Mod::get()->getSettingValue<bool>("show-floating-button")) {
            return true;
        }

        char const* labelText = "AL";
        auto const icon = Mod::get()->getSettingValue<int64_t>("button-icon");
        if (icon == 1) labelText = "*";
        if (icon == 2) labelText = "OK";

        auto label = CCLabelBMFont::create(labelText, "bigFont.fnt");
        if (!label) return true;
        label->setScale(0.55f);

        auto button = CCMenuItemSpriteExtra::create(
            label, this, menu_selector(ALCompletePlayLayer::onALCompleteButton)
        );
        if (!button) return true;

        auto const opacity = Mod::get()->getSettingValue<int64_t>("button-opacity");
        button->setOpacity(static_cast<GLubyte>(std::clamp<int64_t>(opacity, 0, 100) * 255 / 100));

        auto const scale = Mod::get()->getSettingValue<double>("button-scale");
        button->setScale(static_cast<float>(std::clamp(scale, 0.5, 1.5)));
        button->setID("alcomplete-floating-button");

        auto menu = CCMenu::create();
        if (!menu) return true;
        menu->setID("alcomplete-floating-menu");
        menu->addChild(button);

        auto const size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition(ccp(size.width - 42.f, size.height * 0.55f));
        this->addChild(menu, 9999);
        return true;
    }

    void onALCompleteButton(CCObject*) {
        ALCompletePopup::open();
    }
};
