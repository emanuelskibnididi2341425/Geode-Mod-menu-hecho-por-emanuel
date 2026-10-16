#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

// =========================================================
// ESTADO GLOBAL DEL MOD MENU (CONFIGURACION Y HACKS)
// =========================================================
struct ModState {
    // Pestaña Level
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

    // Pestaña Universal / Cosmetic (Personalizar Cubo)
    bool unlockAllIcons = true;
    bool unlockAllColors = true;
    bool practiceMusicHack = true;
    bool noTransition = false;
    bool trailAlwaysOn = false;
    bool trailAlwaysOff = false;

    // Pestaña Speedhack
    bool speedhackEnabled = false;
    float speedhackValue = 1.0f;
    bool speedhackAudio = true;

    // Pestaña Config / Atajos PC
    enumKeyCodes openMenuKey = KEY_Tab;
    enumKeyCodes toggleNoclipKey = KEY_N;
    enumKeyCodes instantRestartKey = KEY_R;

    int currentCategory = 0; // 0: Level, 1: Universal, 2: Cosmetic, 3: Speedhack, 4: Config, 5: Creditos
};

static ModState g_state;

void applySpeedhack() {
    float targetSpeed = g_state.speedhackEnabled ? g_state.speedhackValue : 1.0f;
    if (targetSpeed <= 0.05f) targetSpeed = 0.05f;
    CCDirector::sharedDirector()->getScheduler()->setTimeScale(targetSpeed);
}

// =========================================================
// INTERFAZ GRAFICA DEL MOD MENU (ESTILO QOLMOD / PRISM)
// =========================================================
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
            addCheckboxOption("Noclip", g_state.noclip, menu_selector(EmplayMenuPopup::onToggleOption), 1, {col1X, topY}, "Evita que el jugador muera al chocar con obstaculos.");
            addCheckboxOption("Show Hitboxes", g_state.showHitboxes, menu_selector(EmplayMenuPopup::onToggleOption), 2, {col1X, topY - stepY}, "Muestra las colisiones reales de los objetos.");
            addCheckboxOption("Auto Practice", g_state.autoPracticeMode, menu_selector(EmplayMenuPopup::onToggleOption), 3, {col1X, topY - stepY * 2}, "Activa el modo practica automaticamente al entrar.");
            addCheckboxOption("Coin Tracers", g_state.coinTracers, menu_selector(EmplayMenuPopup::onToggleOption), 4, {col1X, topY - stepY * 3}, "Guia visual hacia las monedas del nivel.");
            addCheckboxOption("Confirm Restart", g_state.confirmRestart, menu_selector(EmplayMenuPopup::onToggleOption), 5, {col1X, topY - stepY * 4}, "Evita reinicios accidentales.");
            addCheckboxOption("Force Hide Player", g_state.forceHidePlayer, menu_selector(EmplayMenuPopup::onToggleOption), 6, {col1X, topY - stepY * 5}, "Hace invisible tu icono durante la partida.");

            addCheckboxOption("Instant Complete", g_state.instantComplete, menu_selector(EmplayMenuPopup::onToggleOption), 7, {col2X, topY}, "Completa el nivel instantaneamente.");
            addCheckboxOption("Hitboxes on Death", g_state.showHitboxesOnDeath, menu_selector(EmplayMenuPopup::onToggleOption), 8, {col2X, topY - stepY}, "Muestra las hitboxes cuando mueres.");
            addCheckboxOption("Icon Kit in Pause", g_state.iconKitInPause, menu_selector(EmplayMenuPopup::onToggleOption), 9, {col2X, topY - stepY * 2}, "Permite personalizar tu cubo desde la pausa.");
            addCheckboxOption("Instant Restart", g_state.instantRestart, menu_selector(EmplayMenuPopup::onToggleOption), 10, {col2X, topY - stepY * 3}, "Reinicia inmediatamente al morir.");
        }
        else if (g_state.currentCategory == 1) {
            addCheckboxOption("Practice Music", g_state.practiceMusicHack, menu_selector(EmplayMenuPopup::onToggleOption), 20, {col1X, topY}, "Usa la musica normal del nivel en el modo practica.");
            addCheckboxOption("No Transition", g_state.noTransition, menu_selector(EmplayMenuPopup::onToggleOption), 21, {col1X, topY - stepY}, "Elimina el fundido en negro entre menus.");
        }
        else if (g_state.currentCategory == 2) {
            addCheckboxOption("Unlock All Icons", g_state.unlockAllIcons, menu_selector(EmplayMenuPopup::onToggleOption), 30, {col1X, topY}, "Desbloquea todos los cubos, naves, UFOs, waves y robots.");
            addCheckboxOption("Unlock All Colors", g_state.unlockAllColors, menu_selector(EmplayMenuPopup::onToggleOption), 31, {col1X, topY - stepY}, "Desbloquea todos los colores y efectos glow para tu cubo.");
            addCheckboxOption("Trail Always On", g_state.trailAlwaysOn, menu_selector(EmplayMenuPopup::onToggleOption), 32, {col1X, topY - stepY * 2}, "Fuerza a que la estela de tu cubo siempre sea visible.");
            addCheckboxOption("Trail Always Off", g_state.trailAlwaysOff, menu_selector(EmplayMenuPopup::onToggleOption), 33, {col2X, topY}, "Oculta completamente la estela del jugador.");

            auto garageSpr = ButtonSprite::create("Abrir Garaje / Personalizar Cubo", "bigFont.fnt", "GJ_button_04.png", 0.55f);
            auto garageBtn = CCMenuItemSpriteExtra::create(garageSpr, this, menu_selector(EmplayMenuPopup::onOpenGarage));
            garageBtn->setPosition({270.f, 95.f});
            m_optionsMenu->addChild(garageBtn);
        }
        else if (g_state.currentCategory == 3) {
            addCheckboxOption("Activar Speedhack", g_state.speedhackEnabled, menu_selector(EmplayMenuPopup::onToggleOption), 40, {col1X, topY}, "Activa o desactiva el cambio de velocidad del juego.");
            addCheckboxOption("Speedhack Audio", g_state.speedhackAudio, menu_selector(EmplayMenuPopup::onToggleOption), 41, {col2X, topY}, "Sincroniza la velocidad de la musica con el speedhack.");

            auto speedLabel = CCLabelBMFont::create(
                fmt::format("Velocidad Actual: {:.2f}x", g_state.speedhackValue).c_str(),
                "goldFont.fnt"
            );
            speedLabel->setScale(0.65f);
            speedLabel->setPosition({275.f, 160.f});
            m_contentLayer->addChild(speedLabel);

            float presets[] = {0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f};
            for (int i = 0; i < 6; i++) {
                auto btnSpr = ButtonSprite::create(
                    fmt::format("{:.2f}x", presets[i]).c_str(),
                    48, true, "bigFont.fnt", "GJ_button_01.png", 25.f, 0.5f
                );
                auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(EmplayMenuPopup::onSelectSpeedPreset));
                btn->setTag(static_cast<int>(presets[i] * 100));
                btn->setPosition({165.f + (i * 44.f), 115.f});
                m_optionsMenu->addChild(btn);
            }
        }
        else if (g_state.currentCategory == 4) {
            auto platformLabel = CCLabelBMFont::create(
                #ifdef GEODE_IS_ANDROID
                "Plataforma: ANDROID (Boton Tactil Activo)"
                #else
                "Plataforma: PC (Atajos de Teclado Activos)"
                #endif
                , "goldFont.fnt"
            );
            platformLabel->setScale(0.45f);
            platformLabel->setPosition({275.f, topY});
            m_contentLayer->addChild(platformLabel);

            auto infoShortcuts = CCLabelBMFont::create(
                "Atajos en PC:\n- TAB: Abrir / Cerrar este Mod Menu\n- Tecla N: Activar / Desactivar Noclip\n- Tecla R: Reinicio rapido del nivel\n\nEn Android: Pulsa el boton flotante 'MOD'\no entra al menu de pausa.",
                "chatFont.fnt"
            );
            infoShortcuts->setScale(0.68f);
            infoShortcuts->setPosition({275.f, 130.f});
            m_contentLayer->addChild(infoShortcuts);
        }
        else if (g_state.currentCategory == 5) {
            auto titleCred = CCLabelBMFont::create("EMPLAY MOD MENU", "goldFont.fnt");
            titleCred->setScale(0.75f);
            titleCred->setPosition({275.f, 195.f});
            m_contentLayer->addChild(titleCred);

            auto authorCred = CCLabelBMFont::create("hecho por emplay1350", "bigFont.fnt");
            authorCred->setScale(0.6f);
            authorCred->setColor({0, 255, 180});
            authorCred->setPosition({275.f, 150.f});
            m_contentLayer->addChild(authorCred);

            auto subCred = CCLabelBMFont::create("Compatible con Geometry Dash PC & Android (Geode)\nDiseno inspirado en QOLMod / MegaHack", "chatFont.fnt");
            subCred->setScale(0.7f);
            subCred->setAlignment(kCCTextAlignmentCenter);
            subCred->setPosition({275.f, 100.f});
            m_contentLayer->addChild(subCred);
        }
    }

    void onToggleOption(CCObject* sender) {
        int tag = sender->getTag();
        switch (tag) {
            case 1: g_state.noclip = !g_state.noclip; break;
            case 2: g_state.showHitboxes = !g_state.showHitboxes; break;
            case 3: g_state.autoPracticeMode = !g_state.autoPracticeMode; break;
            case 4: g_state.coinTracers = !g_state.coinTracers; break;
            case 5: g_state.confirmRestart = !g_state.confirmRestart; break;
            case 6: g_state.forceHidePlayer = !g_state.forceHidePlayer; break;
            case 7: g_state.instantComplete = !g_state.instantComplete; break;
            case 8: g_state.showHitboxesOnDeath = !g_state.showHitboxesOnDeath; break;
            case 9: g_state.iconKitInPause = !g_state.iconKitInPause; break;
            case 10: g_state.instantRestart = !g_state.instantRestart; break;

            case 20: g_state.practiceMusicHack = !g_state.practiceMusicHack; break;
            case 21: g_state.noTransition = !g_state.noTransition; break;

            case 30: g_state.unlockAllIcons = !g_state.unlockAllIcons; break;
            case 31: g_state.unlockAllColors = !g_state.unlockAllColors; break;
            case 32: g_state.trailAlwaysOn = !g_state.trailAlwaysOn; break;
            case 33: g_state.trailAlwaysOff = !g_state.trailAlwaysOff; break;

            case 40:
                g_state.speedhackEnabled = !g_state.speedhackEnabled;
                applySpeedhack();
                break;
            case 41:
                g_state.speedhackAudio = !g_state.speedhackAudio;
                break;
        }
    }

    void onSelectSpeedPreset(CCObject* sender) {
        int val = sender->getTag();
        g_state.speedhackValue = static_cast<float>(val) / 100.0f;
        applySpeedhack();
        refreshCategoryView();
    }

    void onOpenGarage(CCObject*) {
        this->onClose(nullptr);
        CCDirector::sharedDirector()->replaceScene(
            CCTransitionFade::create(0.5f, GJGarageLayer::scene())
        );
    }

public:
    void closeMenu() {
        this->onClose(nullptr);
    }

    static EmplayMenuPopup* create() {
        auto ret = new EmplayMenuPopup();
        if (ret && ret->initAnchored(420.f, 260.f, "GJ_square02.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// =========================================================
// HOOKS DEL JUEGO: NOCLIP, HITBOXES, BOTON ANDROID, ETC.
// =========================================================
class $modify(EmplayPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (g_state.autoPracticeMode) {
            this->togglePracticeMode(true);
        }

        if (Mod::get()->getSettingValue<bool>("show-android-btn")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto menu = CCMenu::create();
            menu->setPosition({0, 0});
            menu->setZOrder(9999);

            auto btnSpr = ButtonSprite::create("MOD", 32, true, "bigFont.fnt", "GJ_button_05.png", 22.f, 0.4f);
            btnSpr->setOpacity(165);
            auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(EmplayPlayLayer::onOpenModMenu));
            btn->setPosition({32.f, winSize.height - 55.f});
            menu->addChild(btn);
            this->addChild(menu);
        }

        return true;
    }

    void onOpenModMenu(CCObject*) {
        EmplayMenuPopup::create()->show();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (m_anticheatSpike && object == m_anticheatSpike) {
            PlayLayer::destroyPlayer(player, object);
            return;
        }

        if (g_state.noclip) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);

        if (g_state.instantRestart && player && player->m_isDead) {
            this->resetLevel();
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (m_player1) {
            m_player1->setVisible(!g_state.forceHidePlayer);
        }
        if (m_player2) {
            m_player2->setVisible(!g_state.forceHidePlayer);
        }

        if (g_state.instantComplete && !m_hasCompletedLevel) {
            this->levelComplete();
        }
    }
};

// =========================================================
// HOOK EN MENU DE PAUSA: BOTON DEL MOD MENU
// =========================================================
class $modify(EmplayPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto spr = ButtonSprite::create("Emplay Menu", 80, true, "bigFont.fnt", "GJ_button_03.png", 26.f, 0.5f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(EmplayPauseLayer::onOpenEmplay));
        btn->setPosition({75.f, winSize.height - 35.f});
        menu->addChild(btn);
    }

    void onOpenEmplay(CCObject*) {
        EmplayMenuPopup::create()->show();
    }
};

// =========================================================
// HOOK EN MENU PRINCIPAL: BOTON ACCESIBLE EN ANDROID & PC
// =========================================================
class $modify(EmplayMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto bottomMenu = this->getChildByID("bottom-menu");
        auto spr = ButtonSprite::create("EMPLAY", 65, true, "goldFont.fnt", "GJ_button_02.png", 30.f, 0.55f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(EmplayMenuLayer::onOpenEmplayMenu));
        btn->setID("emplay-menu-button"_spr);

        if (bottomMenu) {
            bottomMenu->addChild(btn);
            bottomMenu->updateLayout();
        } else {
            auto customMenu = CCMenu::createWithItem(btn);
            customMenu->setPosition({55.f, 280.f});
            this->addChild(customMenu, 100);
        }

        return true;
    }

    void onOpenEmplayMenu(CCObject*) {
        EmplayMenuPopup::create()->show();
    }
};

// =========================================================
// HOOK DE PERSONALIZAR CUBO (ICON & COLOR BYPASS + HITBOXES)
// =========================================================
class $modify(EmplayGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        if (g_state.unlockAllIcons) return true;
        return GameManager::isIconUnlocked(id, type);
    }

    bool isColorUnlocked(int id, UnlockType type) {
        if (g_state.unlockAllColors) return true;
        return GameManager::isColorUnlocked(id, type);
    }

    bool getGameVariable(const char* key) {
        if (!key) return false;
        std::string k(key);
        if (g_state.practiceMusicHack && k == "0071") {
            return true;
        }
        if (g_state.showHitboxes && k == "0166") {
            return true;
        }
        return GameManager::getGameVariable(key);
    }
};

// =========================================================
// HOOK DE TECLADO PARA PC (ATAJOS RAPIDOS)
// =========================================================
class $modify(EmplayKeyboardDispatcher, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
        if (isKeyDown && !isKeyRepeat) {
            if (key == g_state.openMenuKey) {
                auto scene = CCDirector::sharedDirector()->getRunningScene();
                if (scene) {
                    auto existing = scene->getChildByType<EmplayMenuPopup>(0);
                    if (existing) {
                        existing->closeMenu();
                    } else {
                        EmplayMenuPopup::create()->show();
                    }
                }
                return true;
            }

            if (key == g_state.toggleNoclipKey) {
                g_state.noclip = !g_state.noclip;
                Notification::create(
                    g_state.noclip ? "Noclip: ACTIVADO" : "Noclip: DESACTIVADO",
                    NotificationIcon::Info,
                    0.8f
                )->show();
            }

            if (key == g_state.instantRestartKey) {
                if (auto pl = PlayLayer::get()) {
                    pl->resetLevel();
                }
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat);
    }
};
