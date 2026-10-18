#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <string>

using namespace geode::prelude;

class ALFloatingControl;
static ALFloatingControl* g_floatingControl = nullptr;
static void refreshButtonAppearance();
static void performCompletion();

static int clampInt(int value, int low, int high) {
    return std::max(low, std::min(value, high));
}

static CCNode* createALIcon() {
    int iconChoice = clampInt(Mod::get()->getSettingValue<int>("button-icon"), 0, 2);
    const char* iconText = iconChoice == 1 ? "*" : (iconChoice == 2 ? "OK" : "AL");

    auto icon = CCLabelBMFont::create(iconText, "goldFont.fnt");
    if (icon) {
        icon->setScale(0.65f);
    }
    return icon;
}

static CircleButtonSprite* createALCircleButton() {
    auto mod = Mod::get();
    int colorChoice = clampInt(mod->getSettingValue<int>("button-color"), 0, 7);
    int opacity = clampInt(mod->getSettingValue<int>("button-opacity"), 20, 100);
    float scale = std::max(0.5f, std::min(mod->getSettingValue<float>("button-scale"), 1.5f));

    auto icon = createALIcon();
    if (!icon) {
        auto fallback = CircleButtonSprite::createWithSpriteFrameName(
            "GJ_optionsBtn_001.png", 0.8f,
            static_cast<CircleBaseColor>(colorChoice), CircleBaseSize::Medium
        );
        if (fallback) {
            fallback->setScale(scale);
            fallback->setOpacity(static_cast<GLubyte>(opacity * 255 / 100));
        }
        return fallback;
    }

    auto button = CircleButtonSprite::create(
        icon,
        static_cast<CircleBaseColor>(colorChoice),
        CircleBaseSize::Medium
    );

    if (button) {
        button->setCascadeOpacityEnabled(true);
        button->setOpacity(static_cast<GLubyte>(opacity * 255 / 100));
        button->setScale(scale);
    }
    return button;
}

class ALCompletePopup : public Popup {
protected:
    int m_page = 0;
    CCNode* m_pageRoot = nullptr;
    CCMenu* m_tabMenu = nullptr;

    bool init() override {
        if (!Popup::init(340.f, 250.f)) {
            return false;
        }

        this->setTitle("ALComplete");
        this->buildTabs();
        this->buildPage(0);
        return true;
    }

    CCMenuItemSpriteExtra* makeButton(
        const char* text,
        SEL_MenuHandler selector,
        float x,
        float y,
        float scale = 0.58f
    ) {
        auto sprite = ButtonSprite::create(text);
        if (!sprite) {
            return nullptr;
        }

        auto button = CCMenuItemSpriteExtra::create(sprite, this, selector);
        if (!button) {
            return nullptr;
        }

        button->setScale(scale);
        button->setPosition(ccp(x, y));
        return button;
    }

    void addLabel(const std::string& text, float x, float y, float scale = 0.42f) {
        auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        if (!label) {
            return;
        }

        label->setScale(scale);
        label->setPosition(ccp(x, y));
        m_pageRoot->addChild(label);
    }

    void buildTabs() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float cx = winSize.width / 2.f;
        float cy = winSize.height / 2.f;

        m_tabMenu = CCMenu::create();
        if (!m_tabMenu) {
            return;
        }

        m_tabMenu->setPosition(CCPointZero);
        m_tabMenu->setID("alcomplete-tabs");
        this->addChild(m_tabMenu, 20);

        auto quick = makeButton("Actions", menu_selector(ALCompletePopup::onTabActions), cx - 91.f, cy + 55.f, 0.48f);
        auto appearance = makeButton("Appearance", menu_selector(ALCompletePopup::onTabAppearance), cx, cy + 55.f, 0.48f);
        auto behavior = makeButton("Behavior", menu_selector(ALCompletePopup::onTabBehavior), cx + 91.f, cy + 55.f, 0.48f);

        if (quick) m_tabMenu->addChild(quick);
        if (appearance) m_tabMenu->addChild(appearance);
        if (behavior) m_tabMenu->addChild(behavior);
    }

    void buildPage(int page) {
        m_page = page;
        if (m_pageRoot) {
            m_pageRoot->removeFromParentAndCleanup(true);
            m_pageRoot = nullptr;
        }

        m_pageRoot = CCNode::create();
        if (!m_pageRoot) {
            return;
        }
        m_pageRoot->setID("alcomplete-page");
        this->addChild(m_pageRoot, 10);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float cx = winSize.width / 2.f;
        float cy = winSize.height / 2.f;

        auto menu = CCMenu::create();
        if (!menu) {
            return;
        }
        menu->setPosition(CCPointZero);
        m_pageRoot->addChild(menu);

        if (page == 0) {
            auto* playLayer = PlayLayer::get();
            if (playLayer) {
                addLabel(
                    "Current progress: " + std::to_string(playLayer->getCurrentPercentInt()) + "%",
                    cx, cy + 20.f, 0.44f
                );
                addLabel("Uses the game's native finish routine.", cx, cy + 1.f, 0.34f);
            } else {
                addLabel("Start a level to use Instant Complete.", cx, cy + 18.f, 0.42f);
                addLabel("Appearance and behavior can be changed here.", cx, cy - 1.f, 0.32f);
            }

            auto complete = makeButton(
                "Complete Level",
                menu_selector(ALCompletePopup::onCompleteLevel),
                cx,
                cy - 38.f,
                0.70f
            );
            if (complete) menu->addChild(complete);
        } else if (page == 1) {
            auto mod = Mod::get();
            int opacity = mod->getSettingValue<int>("button-opacity");
            float scale = mod->getSettingValue<float>("button-scale");
            int icon = mod->getSettingValue<int>("button-icon");
            int color = mod->getSettingValue<int>("button-color");

            addLabel("Opacity: " + std::to_string(opacity) + "%", cx - 64.f, cy + 23.f, 0.38f);
            addLabel("Scale: " + std::to_string(scale).substr(0, 4), cx + 64.f, cy + 23.f, 0.38f);
            addLabel(
                "Icon: " + std::string(icon == 1 ? "*" : (icon == 2 ? "OK" : "AL")) +
                    "   Color preset: " + std::to_string(color),
                cx, cy + 3.f, 0.35f
            );

            auto opacityButton = makeButton("Opacity +", menu_selector(ALCompletePopup::onCycleOpacity), cx - 64.f, cy - 25.f, 0.50f);
            auto scaleButton = makeButton("Scale +", menu_selector(ALCompletePopup::onCycleScale), cx + 64.f, cy - 25.f, 0.50f);
            auto iconButton = makeButton("Cycle Icon", menu_selector(ALCompletePopup::onCycleIcon), cx - 64.f, cy - 67.f, 0.50f);
            auto colorButton = makeButton("Cycle Color", menu_selector(ALCompletePopup::onCycleColor), cx + 64.f, cy - 67.f, 0.50f);

            if (opacityButton) menu->addChild(opacityButton);
            if (scaleButton) menu->addChild(scaleButton);
            if (iconButton) menu->addChild(iconButton);
            if (colorButton) menu->addChild(colorButton);
        } else {
            auto mod = Mod::get();
            bool confirm = mod->getSettingValue<bool>("confirm-before-complete");
            bool snap = mod->getSettingValue<bool>("snap-to-edge");
            bool visible = mod->getSettingValue<bool>("show-floating-button");

            addLabel(std::string("Confirmation: ") + (confirm ? "ON" : "OFF"), cx, cy + 23.f, 0.40f);
            addLabel(std::string("Edge snap: ") + (snap ? "ON" : "OFF"), cx, cy + 3.f, 0.40f);
            addLabel(std::string("Floating button: ") + (visible ? "ON" : "OFF"), cx, cy - 17.f, 0.40f);

            auto confirmButton = makeButton("Toggle Confirm", menu_selector(ALCompletePopup::onToggleConfirm), cx - 70.f, cy - 57.f, 0.48f);
            auto snapButton = makeButton("Toggle Snap", menu_selector(ALCompletePopup::onToggleSnap), cx + 70.f, cy - 57.f, 0.48f);
            auto visibleButton = makeButton("Toggle Button", menu_selector(ALCompletePopup::onToggleVisibility), cx, cy - 91.f, 0.48f);

            if (confirmButton) menu->addChild(confirmButton);
            if (snapButton) menu->addChild(snapButton);
            if (visibleButton) menu->addChild(visibleButton);
        }
    }

public:
    static ALCompletePopup* create() {
        auto ret = new ALCompletePopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void onTabActions(CCObject*) {
        buildPage(0);
    }

    void onTabAppearance(CCObject*) {
        buildPage(1);
    }

    void onTabBehavior(CCObject*) {
        buildPage(2);
    }

    void onCompleteLevel(CCObject*) {
        if (!PlayLayer::get()) {
            FLAlertLayer::create(
                "ALComplete",
                "There is no active level. Start a level first, then reopen ALComplete.",
                "OK"
            )->show();
            return;
        }

        this->onClose(nullptr);

        if (Mod::get()->getSettingValue<bool>("confirm-before-complete")) {
            createQuickPopup(
                "ALComplete",
                "Instantly call Geometry Dash's level-complete routine? This may affect local progress and does not guarantee a verified leaderboard record.",
                "Cancel",
                "Complete",
                [](FLAlertLayer*, bool confirm) {
                    if (confirm) {
                        performCompletion();
                    }
                }
            );
        } else {
            performCompletion();
        }
    }

    void onCycleOpacity(CCObject*) {
        auto mod = Mod::get();
        int value = mod->getSettingValue<int>("button-opacity");
        value = value >= 100 ? 20 : std::min(100, value + 10);
        mod->setSettingValue<int>("button-opacity", value);
        refreshButtonAppearance();
        buildPage(1);
    }

    void onCycleScale(CCObject*) {
        auto mod = Mod::get();
        float value = mod->getSettingValue<float>("button-scale");
        value = value >= 1.5f ? 0.5f : std::min(1.5f, value + 0.1f);
        mod->setSettingValue<float>("button-scale", value);
        refreshButtonAppearance();
        buildPage(1);
    }

    void onCycleIcon(CCObject*) {
        auto mod = Mod::get();
        int value = mod->getSettingValue<int>("button-icon");
        mod->setSettingValue<int>("button-icon", (value + 1) % 3);
        refreshButtonAppearance();
        buildPage(1);
    }

    void onCycleColor(CCObject*) {
        auto mod = Mod::get();
        int value = mod->getSettingValue<int>("button-color");
        mod->setSettingValue<int>("button-color", (value + 1) % 8);
        refreshButtonAppearance();
        buildPage(1);
    }

    void onToggleConfirm(CCObject*) {
        auto mod = Mod::get();
        mod->setSettingValue<bool>(
            "confirm-before-complete",
            !mod->getSettingValue<bool>("confirm-before-complete")
        );
        buildPage(2);
    }

    void onToggleSnap(CCObject*) {
        auto mod = Mod::get();
        mod->setSettingValue<bool>(
            "snap-to-edge",
            !mod->getSettingValue<bool>("snap-to-edge")
        );
        buildPage(2);
    }

    void onToggleVisibility(CCObject*) {
        auto mod = Mod::get();
        mod->setSettingValue<bool>(
            "show-floating-button",
            !mod->getSettingValue<bool>("show-floating-button")
        );
        refreshButtonAppearance();
        buildPage(2);
    }
};

static void performCompletion() {
    auto playLayer = PlayLayer::get();
    if (!playLayer) {
        FLAlertLayer::create(
            "ALComplete",
            "No active level was found. Nothing was completed.",
            "OK"
        )->show();
        return;
    }

    log::info("ALComplete: calling PlayLayer::levelComplete()");
    playLayer->levelComplete();
}

class ALFloatingControl : public CCLayer {
protected:
    CCNode* m_visual = nullptr;
    CCPoint m_touchStart = CCPointZero;
    CCPoint m_positionStart = CCPointZero;
    bool m_touching = false;
    bool m_moved = false;

    bool init() {
        if (!CCLayer::init()) {
            return false;
        }

        this->setContentSize(CCSizeMake(64.f, 64.f));
        this->setAnchorPoint(ccp(0.5f, 0.5f));
        this->setTouchEnabled(true);
        this->refreshAppearance();
        return true;
    }

public:
    static ALFloatingControl* create() {
        auto ret = new ALFloatingControl();
        if (ret && ret->init()) {
            ret->autorelease();
            g_floatingControl = ret;
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void refreshAppearance() {
        if (m_visual) {
            m_visual->removeFromParentAndCleanup(true);
            m_visual = nullptr;
        }

        auto visual = createALCircleButton();
        if (!visual) {
            visual = CircleButtonSprite::createWithSpriteFrameName(
                "GJ_optionsBtn_001.png", 0.8f,
                CircleBaseColor::Green, CircleBaseSize::Medium
            );
        }

        if (visual) {
            visual->setPosition(ccp(32.f, 32.f));
            m_visual = visual;
            this->addChild(m_visual);
        }

        this->setVisible(Mod::get()->getSettingValue<bool>("show-floating-button"));
    }

    void onExit() override {
        if (g_floatingControl == this) {
            g_floatingControl = nullptr;
        }
        CCLayer::onExit();
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        if (!this->isVisible()) {
            return false;
        }

        auto local = this->convertToNodeSpace(touch->getLocation());
        if (local.x < 0.f || local.y < 0.f ||
            local.x > 64.f || local.y > 64.f) {
            return false;
        }

        m_touching = true;
        m_moved = false;
        m_touchStart = touch->getLocation();
        m_positionStart = this->getPosition();
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_touching) {
            return;
        }

        auto location = touch->getLocation();
        float dx = location.x - m_touchStart.x;
        float dy = location.y - m_touchStart.y;

        if ((dx * dx + dy * dy) > 64.f) {
            m_moved = true;
        }

        if (!m_moved) {
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float x = std::max(32.f, std::min(winSize.width - 32.f, m_positionStart.x + dx));
        float y = std::max(32.f, std::min(winSize.height - 32.f, m_positionStart.y + dy));
        this->setPosition(ccp(x, y));
    }

    void ccTouchEnded(CCTouch*, CCEvent*) override {
        if (!m_touching) {
            return;
        }

        m_touching = false;

        if (!m_moved) {
            auto popup = ALCompletePopup::create();
            if (popup) {
                popup->show();
            }
            return;
        }

        auto mod = Mod::get();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto position = this->getPosition();

        if (mod->getSettingValue<bool>("snap-to-edge")) {
            position.x = position.x < winSize.width / 2.f
                ? 34.f
                : winSize.width - 34.f;
            this->setPosition(position);
        }

        mod->setSavedValue<float>("floating-button-x", position.x);
        mod->setSavedValue<float>("floating-button-y", position.y);
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) override {
        m_touching = false;
        m_moved = false;
    }
};

static void refreshButtonAppearance() {
    if (g_floatingControl) {
        g_floatingControl->refreshAppearance();
    }
}

class $modify(ALCompleteMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto sprite = createALCircleButton();
        if (!sprite) {
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(ALCompleteMenuLayer::onALComplete)
        );
        if (!button) {
            return true;
        }

        button->setID("alcomplete-main-menu-button");
        button->setPosition(ccp(winSize.width - 105.f, winSize.height - 75.f));

        auto menu = CCMenu::create();
        if (!menu) {
            return true;
        }

        menu->setPosition(CCPointZero);
        menu->addChild(button);
        menu->setID("alcomplete-main-menu");
        this->addChild(menu, 1000);
        return true;
    }

    void onALComplete(CCObject*) {
        auto popup = ALCompletePopup::create();
        if (popup) {
            popup->show();
        }
    }
};

class $modify(ALCompletePlayLayer, PlayLayer) {
    void onEnterTransitionDidFinish() {
        PlayLayer::onEnterTransitionDidFinish();

        if (!Mod::get()->getSettingValue<bool>("show-floating-button")) {
            return;
        }

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene || scene->getChildByID("alcomplete-floating-control")) {
            return;
        }

        auto control = ALFloatingControl::create();
        if (!control) {
            log::error("ALComplete: failed to create the floating control.");
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float x = Mod::get()->getSavedValue<float>("floating-button-x", winSize.width - 52.f);
        float y = Mod::get()->getSavedValue<float>("floating-button-y", winSize.height * 0.55f);
        x = std::max(32.f, std::min(winSize.width - 32.f, x));
        y = std::max(32.f, std::min(winSize.height - 32.f, y));

        control->setPosition(ccp(x, y));
        control->setID("alcomplete-floating-control");
        scene->addChild(control, 9999);
        log::info("ALComplete: floating control added to gameplay scene.");
    }
};
