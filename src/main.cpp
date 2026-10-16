#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

struct ModState {
    bool noclip = false;
    bool instantComplete = false;
    bool showHitboxes = false;
    bool showHitboxesOnDeath = true;
    bool autoPracticeMode = false;
    bool coinTracers = false;
    bool confirmRestart = false;
    bool forceHidePlayer = false;
    bool iconKitInPause = true;
    bool instantRestart = false;

    bool unlockAllIcons = true;
    bool unlockAllColors = true;
    bool practiceMusicHack = true;
    bool noTransition = false;
    bool trailAlwaysOn = false;
    bool trailAlwaysOff = false;

    bool speedhackEnabled = false;
    float speedhackValue = 1.0f;
    bool speedhackAudio = true;

    enumKeyCodes openMenuKey = KEY_Tab;
    enumKeyCodes toggleNoclipKey = KEY_N;
    enumKeyCodes instantRestartKey = KEY_R;

    int currentCategory = 0;
};

static ModState g_state;

void applySpeedhack() {
    float targetSpeed = g_state.speedhackEnabled ? g_state.speedhackValue : 1.0f;
    if (targetSpeed <= 0.05f) targetSpeed = 0.05f;
    CCDirector::sharedDirector()->getScheduler()->setTimeScale(targetSpeed);
}

class EmplayMenuPopup : public geode::Popup<> {
protected:
    CCLayer* m_contentLayer = nullptr;
    CCMenu* m_categoryMenu = nullptr;
    CCMenu* m_optionsMenu = nullptr;

    bool setup() override {
        this->setTitle("Emplay Mod Menu");

        auto bgPanel = CCLayerColor::create({25, 30, 40, 200}, 390.f, 215.f);
        bgPanel->setPosition({m_mainLayer->getContentSize().width / 2 - 195.f, m_mainLayer->getContentSize().height / 2 - 115.f});
        m_mainLayer->addChild(bgPanel, -1);

        auto sideBarBg = CCLayerColor::create({15, 20, 28, 230}, 105.f, 215.f);
        sideBarBg->setPosition(bgPanel->getPosition());
        m_mainLayer->addChild(sideBarBg, 0);

        m_categoryMenu = CCMenu::create();
        m_categoryMenu->setPosition({0, 0});
        m_mainLayer->addChild(m_categoryMenu, 2);

        createCategoryButtons();

        m_contentLayer = CCLayer::create();
        m_mainLayer->addChild(m_contentLayer, 2);

        refreshCategoryView();

        return true;
    }

    void createCategoryButtons() {
        m_categoryMenu->removeAllChildren();

        const char* categories[] = {
            "Level",
            "Universal",
            "Cosmetic",
            "Speedhack",
            "Config / PC",
            "Creditos"
        };

        float startY = 215.f;
        float startX = 82.f;

        for (int i = 0; i < 6; i++) {
            bool selected = (g_state.currentCategory == i);
            auto spr = ButtonSprite::create(
                categories[i],
                85,
                true,
                "bigFont.fnt",
                selected ? "GJ_button_02.png" : "GJ_button_01.png",
                28.f,
                0.55f
            );
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(EmplayMenuPopup::onSelectCategory)
            );
            btn->setTag(i);
            btn->setPosition({startX, startY - (i * 33.f)});
            m_categoryMenu->addChild(btn);
        }
    }

    void onSelectCategory(CCObject* sender) {
        g_state.currentCategory = sender->getTag();
        createCategoryButtons();
        refreshCategoryView();
    }

    void addCheckboxOption(const char* labelText, bool currentValue, SEL_MenuHandler callback, int tag, CCPoint pos, const char* infoDesc = nullptr) {
        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, callback, 0.65f
        );
        toggler->toggle(currentValue);
        toggler->setTag(tag);
        toggler->setPosition(pos);
        m_optionsMenu->addChild(toggler);

        auto label = CCLabelBMFont::create(labelText, "bigFont.fnt");
        label->setScale(0.35f);
        label->setAnchorPoint({0.f, 0.5f});
        label->setPosition({pos.x + 18.f, pos.y});
        m_contentLayer->addChild(label);

        if (infoDesc) {
            auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            infoSpr->setScale(0.55f);
            auto infoBtn = CCMenuItemSpriteExtra::create(
                infoSpr, this, menu_selector(EmplayMenuPopup::onShowInfo)
            );
            infoBtn->setUserObject(CCString::create(infoDesc));
            infoBtn->setPosition({pos.x + 18.f + label->getScaledContentSize().width + 10.f, pos.y});
            m_optionsMenu->addChild(infoBtn);
        }
    }

    void onShowInfo(CCObject* sender) {
        auto node = static_cast<CCNode*>(sender);
        auto str = static_cast<CCString*>(node->getUserObject());
        if (str) {
            FLAlertLayer::create("Info del Mod", str->getCString(), "OK")->show();
        }
    }

    void refreshCategoryView() {
        m_contentLayer->removeAllChildren();
        m_optionsMenu = CCMenu::create();
        m_optionsMenu->setPosition({0, 0});
        m_contentLayer->addChild(m_optionsMenu);

        float col1X = 152.f;
        float col2X = 285.f;
        float topY = 215.f;
        float stepY = 30.f;

        if (g_state.currentCategory == 0) {
            addCheckboxOption("Noclip", g_state.noclip, menu_selector(EmplayMenuPopup::onToggleOption), 1, {col1X, topY}, "Evita que el jugador muera al chocar.");
            addCheckboxOption("Show Hitboxes", g_state.showHitboxes, menu_selector(EmplayMenuPopup::onToggleOption), 2, {col1X, topY - stepY}, "Muestra las colisiones reales.");
            addCheckboxOption("Auto Practice", g_state.autoPracticeMode, menu_selector(EmplayMenuPopup::onToggleOption), 3, {col1X, topY - stepY * 2}, "Activa practica automaticamente.");
            addCheckboxOption("Coin Tracers", g_state.coinTracers, menu_selector(EmplayMenuPopup::onToggleOption), 4, {col1X, topY - stepY * 3}, "Guia hacia las monedas.");
            addCheckboxOption("Confirm Restart", g_state.confirmRestart, menu_selector(EmplayMenuPopup::onToggleOption), 5, {col1X, topY - stepY * 4}, "Evita reinicios accidentales.");
            addCheckboxOption("Force Hide Player", g_state.forceHidePlayer, menu_selector(EmplayMenuPopup::onToggleOption), 6, {col1X, topY - stepY * 5}, "Oculta tu icono en partida.");

            addCheckboxOption("Instant Complete", g_state.instantComplete, menu_selector(EmplayMenuPopup::onToggleOption), 7, {col2X, topY}, "Completa el nivel al instante.");
            addCheckboxOption("Hitboxes on Death", g_state.showHitboxesOnDeath, menu_selector(EmplayMenuPopup::onToggleOption), 8, {col2X, topY - stepY}, "Hitboxes al morir.");
            addCheckboxOption("Icon Kit in Pause", g_state.iconKitInPause, menu_selector(EmplayMenuPopup::onToggleOption), 9, {col2X, topY - stepY * 2}, "Personaliza tu cubo en pausa.");
            addCheckboxOption("Instant Restart", g_state.instantRestart, menu_selector(EmplayMenuPopup::onToggleOption), 10, {col2X, topY - stepY * 3}, "Reinicio inmediato al morir.");
        }
        else if (g_state.currentCategory == 1) {
            addCheckboxOption("Practice Music", g_state.practiceMusicHack, menu_selector(EmplayMenuPopup::onToggleOption), 20, {col1X, topY}, "Musica normal en modo practica.");
            addCheckboxOption("No Transition", g_state.noTransition, menu_selector(EmplayMenuPopup::onToggleOption), 21, {col1X, topY - stepY}, "Quita fundidos en negro.");
        }
        else if (g_state.currentCategory == 2) {
            addCheckboxOption("Unlock All Icons", g_state.unlockAllIcons, menu_selector(EmplayMenuPopup::onToggleOption), 30, {col1X, topY}, "Desbloquea todos los cubos y naves.");
            addCheckboxOption("Unlock All Colors", g_state.unlockAllColors, menu_selector(EmplayMenuPopup::onToggleOption), 31, {col1X, topY - stepY}, "Desbloquea colores y glow.");
            addCheckboxOption("Trail Always On", g_state.trailAlwaysOn, menu_selector(EmplayMenuPopup::onToggleOption), 32, {col1X, topY - stepY * 2}, "Estela siempre visible.");
            addCheckboxOption("Trail Always Off", g_state.trailAlwaysOff, menu_selector(EmplayMenuPopup::onToggleOption), 33, {col2X, topY}, "
