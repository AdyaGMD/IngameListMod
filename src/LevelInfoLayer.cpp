#include "include.h"
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include "json.h"

std::unordered_map<int, int> cachedPositions;
bool requestfinished = false;

class DemonClass {
public:
    void infobox(CCObject*);
    void openLink(cocos2d::CCObject* ret);
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

    std::string domain = pointercrate->getValue() ? "https://pointercrate.com/demonlist/" : "https://challengelist.gd/challenges/";
    if (platformer) domain = "https://www.platformerlist.com/demons/";
    std::string url = domain + std::string(std::to_string(position->getValue()));
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
}

void infoButton(CCLayer* layer, CCLabelBMFont* thelabel) {
    CCPoint position = { thelabel->getPositionX() - 122, thelabel->getPositionY() - 81 };
    CCSprite* buttonbg = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    auto button = CCMenuItemSpriteExtra::create(buttonbg, layer, menu_selector(DemonClass::infobox));
    auto menu = CCMenu::create();
    menu->setScale(0.5f);
    menu->addChild(button);
    menu->setPosition(position);
    layer->addChild(menu);
}

void DemonClass::infobox(CCObject*) {
    FLAlertLayer::create("N/A Position Help", "The <cr>Demon</c> or <cr>Challenge</c> has either never been <cl>List Worthy</c> or hasn't been placed yet on the <cy>List</c>.", "OK")->show();
}

void getRequest(CCLayer* self, GJGameLevel* level, CCLabelBMFont* thelabel, bool pointercrate, bool platformer)
{
    static nlohmann::json childJson;

    self->retain();

    std::string lvlname = level->m_levelName;
    std::string lvlID = std::to_string(level->m_levelID);
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

    std::string url = pointercrate ? "https://pointercrate.com/api/v2/demons/listed/?name=" + std::string(lvlname) : "https://challengelist.gd/api/v2/demons/listed/?name=" + std::string(lvlname);
    if (platformer) url = "https://www.platformerlist.com/api/demon/?level_id=" + std::string(lvlID);

    web::AsyncWebRequest()
        .fetch(url)
        .text()
        .then([self, thelabel, pointercrate, level, platformer](std::string const& resultat) mutable {
            std::cout << resultat << "\n\n";

            std::string result = "[" + resultat + "]";

            childJson = nlohmann::json::parse(result);

            self->autorelease();

            if (childJson[0].contains("position")) {
                int position = childJson[0]["position"];
                std::string label = std::string(std::to_string(position));
                thelabel->setString(label.c_str());
                CCDictionary* pos = CCDictionary::create();
                pos->setObject(CCInteger::create(position), "get");
                pos->setObject(CCBool::create(pointercrate), "domain");
                pos->setObject(CCBool::create(platformer), "platformer");
                createButton(self, thelabel, pos, pointercrate, platformer);
                cachedPositions.insert({ level->m_levelID, position });
            }
            else {
                thelabel->setString("N/A");
                infoButton(self, thelabel);
                cachedPositions.insert({ level->m_levelID, -1 });
            }
        })
        .expect([](std::string const& error) {
            return;
        });

    return;
}

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool idk) {
    if (!LevelInfoLayer::init(level, idk)) return false;

    bool platformer = false;
    if (level->m_demon == 1 && level->m_levelLength == 5) platformer = true;
    if (level->m_ratingsSum < 40 && !platformer) return true;
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
        getRequest(this, level, thelabel, pointercrate, platformer);
    }

    addChild(thelabel);

    return true;
    }
};