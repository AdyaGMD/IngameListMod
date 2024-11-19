#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Loader.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

std::unordered_map<int, int> cachedPositions;
bool requestfinished = false;
bool partitied = false;
int listtype;

EventListener<web::WebTask> webreq;

class DemonClass {
public:
    void infobox(CCObject* sender);
    void internetFail(CCObject* sender);
    void openLink(CCObject* ret);
};

static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

void createButton(CCLayer* self, CCLabelBMFont* thelabel, CCDictionary* pos, bool pointercrate, bool platformer) {
    CCPoint position = { thelabel->getPositionX() + 8.f, thelabel->getPositionY() };
    listtype = 1;
    if (pointercrate) listtype = 0;
    if (platformer) listtype = 2;
    auto button = CCMenuItemSpriteExtra::create(thelabel, self, menu_selector(DemonClass::openLink));
    button->setUserObject(pos);
    auto menu = CCMenu::create(); // To make the button "clickable"
    auto texture = "rankIcon_top50_001.png";
    CCSprite* trophy;
    if (pointercrate) texture = "rankIcon_top10_001.png";
    if (platformer) texture = "rankIcon_top500_001.png";
    trophy = CCSprite::createWithSpriteFrameName(texture);
    (platformer) ? trophy->setScale(0.8f) : trophy->setScale(0.5f);
    trophy->setPosition({-10.f , 5.f});
    menu->addChild(button);
    menu->setPosition(position);
    button->addChild(trophy);
    self->addChild(menu);
}

void DemonClass::openLink(CCObject* ret) {
    CCDictionary* dict = static_cast<CCDictionary*>(static_cast<CCNode*>(ret)->getUserObject());
    CCInteger* position = reinterpret_cast<CCInteger*>(dict->objectForKey("get"));
    CCBool* pointercrate = reinterpret_cast<CCBool*>(dict->objectForKey("domain"));
    CCBool* platformer = reinterpret_cast<CCBool*>(dict->objectForKey("platformer"));
    if (listtype > 0) {
        std::string domain = "https://challengelist.gd/challenges/";
        if (listtype == 2) domain = "https://pemonlist.com/level/";
        std::string url = domain + std::string(std::to_string(position->getValue()));
        web::openLinkInBrowser(url.c_str());
    }
}

void infoButton(CCLayer* layer, CCLabelBMFont* thelabel, bool internetFail = false) {
    CCPoint position = { thelabel->getPositionX() - 122, thelabel->getPositionY() - 81 };
    CCSprite* buttonbg = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    auto button = CCMenuItemSpriteExtra::create(buttonbg, layer, menu_selector(DemonClass::infobox));
    if (internetFail) button = CCMenuItemSpriteExtra::create(buttonbg, layer, menu_selector(DemonClass::internetFail));
    auto menu = CCMenu::create();
    menu->setScale(0.5f);
    menu->addChild(button);
    menu->setPosition(position);
    layer->addChild(menu);
}

void DemonClass::infobox(CCObject* sender) {
    FLAlertLayer::create("N/A Position Help", "The <cr>Demon</c> or <cr>Challenge</c> has either never been <cl>List Worthy</c> or hasn't been placed yet on the <cy>List</c>.", "OK")->show();
}

void DemonClass::internetFail(CCObject* sender) {
    FLAlertLayer::create("??? Position Help", "IngameListMod is unable to find the ranking of this level right now.\n\n<cy>This is usually not your fault, but double-check your Internet connection just to be safe.</c>\n\nPing me in the Geode SDK Discord server (<cb>@adyagd</c>) if you continue seeing this error.", "OK")->show();
}

void getRequest(CCLayer* self, GJGameLevel* level, CCLabelBMFont* thelabel, bool pointercrate, bool platformer)
{
    static matjson::Value childJson;

    self->retain();

    std::string lvlname = level->m_levelName;
    int lvlID = level->m_levelID;
    std::string oldStr = " ";
    std::string newStr = "%20";

    while (!lvlname.empty() && lvlname.back() == ' ') {
        lvlname.erase(lvlname.length() - 1);
    }

    for (size_t pos = 0; ; pos += newStr.length()) {
        pos = lvlname.find(oldStr, pos);
        if (pos == std::string::npos) break;
        lvlname.replace(pos, oldStr.length(), newStr);
    }

    std::string positionstring = "position";

    std::string url = pointercrate ? "https://api.aredl.net/api/aredl/levels/" + std::to_string(lvlID) + "?two_player=false&records=false&creators=false&verification=false&packs=false" : "https://challengelist.gd/api/v2/demons/listed/?name=" + std::string(lvlname);
    if (platformer) url = "https://pemonlist.com/api/level/" + std::to_string(lvlID);
    if (platformer) positionstring = "placement";

    webreq.bind([self, thelabel, pointercrate, level, platformer, positionstring, lvlID](web::WebTask::Event* e) mutable {
        if (web::WebResponse* res = e->getValue()) {
            std::string resultat = res->string().unwrap();
            log::info("{}\n\n", resultat);
            std::string result;
            if (!platformer && !pointercrate) {
                result = resultat;
            } else {
                result = "[" + resultat + "]";
            }
            // Parse the result using matjson
            auto parseResult = matjson::parse(result);
            if (!parseResult) {
                log::info("Failed to parse JSON: {}", parseResult.unwrapErr());
                thelabel->setString("???");
                infoButton(self, thelabel, true);
                return;
            }
            childJson = parseResult.unwrap();

            self->autorelease();

            matjson::Value child;

            if (childJson.isArray() && childJson.size() > 0) {
                child = childJson[0];
            } else if (childJson.isObject()) {
                child = childJson;
            } else {
                thelabel->setString("N/A");
                infoButton(self, thelabel);
                cachedPositions.insert({ level->m_levelID, -1 });
                return;
            }

            if (child.isObject() && child.contains(positionstring)) {
                auto positionValue = child[positionstring];
                auto positionResult = positionValue.asInt();
                if (!positionResult) {
                    log::info("Expected position to be an int");
                    thelabel->setString("???");
                    infoButton(self, thelabel, true);
                    return;
                }
                int position = positionResult.unwrap();
                log::info("{}", position);
                std::string label = std::to_string(position);
                thelabel->setString(label.c_str());
                CCDictionary* pos = CCDictionary::create();
                pos->setObject(CCInteger::create(lvlID), "get");
                pos->setObject(CCBool::create(pointercrate), "domain");
                pos->setObject(CCBool::create(platformer), "platformer");
                createButton(self, thelabel, pos, pointercrate, platformer);
                cachedPositions.insert({ level->m_levelID, position });
            } else {
                thelabel->setString("N/A");
                infoButton(self, thelabel);
                cachedPositions.insert({ level->m_levelID, -1 });
            }
        } else if (e->isCancelled()) {
            thelabel->setString("???");
            infoButton(self, thelabel, true);
        }
    });

    auto req = web::WebRequest();
    webreq.setFilter(req.get(url));

    return;
}

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool idk) {
    if (!LevelInfoLayer::init(level, idk)) return false;

    bool platformer = false;
    if (level->m_demon == 1 && level->m_levelLength == 5) platformer = true;
    if (level->m_ratingsSum < 50 && !platformer) return true;
    if (level->m_demon != 1 && level->m_levelLength >= 2) return true;
    if (platformer) log::info("platformer"); 

    int offset = (level->m_coins == 0) ? 17 : 4;

    auto director = CCDirector::sharedDirector();
    auto size = director->getWinSize();
    auto it = cachedPositions.find(level->m_levelID);

    bool pointercrate = (level->m_stars == 10) ? true : false;

    int yoffset = pointercrate ? 0 : 7.f;

    CCLabelBMFont* thelabel;
    thelabel = CCLabelBMFont::create(" ... ", "goldFont.fnt");
    
    thelabel->setPosition({ size.width / 2 - 100, size.height / 2 + offset + yoffset });
    thelabel->setScale(0.5f);

    if (!pointercrate || platformer) {
        thelabel->setFntFile("bigFont.fnt");
        thelabel->setScale(0.4f);
    }

    if (it != cachedPositions.end()) {
        if (cachedPositions[level->m_levelID] > -1) {
            int position = cachedPositions[level->m_levelID];
            std::string label = std::string(std::to_string(position));
            thelabel->setString(label.c_str());
            CCDictionary* pos = CCDictionary::create();
            pos->setObject(CCBool::create(pointercrate), "domain");
            pos->setObject(CCInteger::create(position), "get");
            pos->setObject(CCBool::create(platformer), "platformer");
            createButton(this, thelabel, pos, pointercrate, platformer);
        }
        else {
            thelabel->setString("N/A");      
            infoButton(this, thelabel);
        }
    }
    else {
        log::info("retard");
        getRequest(this, level, thelabel, pointercrate, platformer);
    }

    addChild(thelabel);

    return true;
    }
};

class $modify(MenuLayer) {
    bool init() {
        auto result = MenuLayer::init();
        if (partitied) return result;
        Loader* loader = Loader::get();
        std::vector<Mod*> mods = loader->getAllMods();
        if (loader->isModLoaded("gdutilsdevs.gdutils")) {
            loader->getLoadedMod("gdutilsdevs.gdutils")->setSettingValue<bool>("demonListPlacement", false);
        }
        partitied = true;
        return result;
    }
};
