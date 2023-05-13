#define CURL_STATICLIB

#include "pch.h"
#include "LevelInfoLayer.h"
#include "curl/curl.h"
#include <thread>
#include "stdio.h"
#include "json.hpp"
#include <windows.h>
#include <shellapi.h>

std::unordered_map<int, int> cachedPositions;
bool requestfinished = false;

static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

void createButton(CCLayer* self, CCLabelBMFont* thelabel, CCDictionary* pos, bool pointercrate) {
    CCPoint position = { thelabel->getPositionX() + 8.f, thelabel->getPositionY() };
    auto button = gd::CCMenuItemSpriteExtra::create(thelabel, self, menu_selector(LevelInfoLayer::openLink));
    button->setUserObject(pos);
    auto menu = CCMenu::create(); // To make the button "clickable"
    auto texture = "rankIcon_top50_001.png";
    CCSprite* trophy;
    if (pointercrate) texture = "rankIcon_top10_001.png";
    trophy = CCSprite::createWithSpriteFrameName(texture);
    trophy->setScale(0.5f);
    trophy->setPosition({-10.f , 5.f});
    menu->addChild(button);
    menu->setPosition(position);
    button->addChild(trophy);
    self->addChild(menu);
}

void LevelInfoLayer::openLink(CCObject* ret) {
    CCDictionary* dict = static_cast<CCDictionary*>(static_cast<CCNode*>(ret)->getUserObject());
    CCInteger* position = reinterpret_cast<CCInteger*>(dict->objectForKey("get"));
    CCBool* pointercrate = reinterpret_cast<CCBool*>(dict->objectForKey("domain"));

    std::string domain = pointercrate->getValue() ? "https://pointercrate.com/demonlist/" : "https://challengelist.gd/challenges/";
    std::string url = domain + std::string(std::to_string(position->getValue()));
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
}

void infoButton(CCLayer* layer, CCLabelBMFont* thelabel) {
    CCPoint position = { thelabel->getPositionX() - 122, thelabel->getPositionY() - 81};

    CCSprite* buttonbg = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");

    auto button = gd::CCMenuItemSpriteExtra::create(buttonbg, layer, menu_selector(LevelInfoLayer::infobox));
    auto menu = CCMenu::create();

    menu->setScale(0.5f);
    menu->addChild(button);
    menu->setPosition(position);
    layer->addChild(menu);
}

void LevelInfoLayer::infobox(CCObject*) {
    gd::FLAlertLayer::create(nullptr, "N/A Position Help", "OK", nullptr, "The <cr>Demon</c> or <cr>Challenge</c> has either never been <cl>List Worthy</c> or hasn't been placed yet on the <cy>List</c>.")->show();
}

void onHttpRequestCompleted(CCLayer* self, gd::GJGameLevel* level, CCLabelBMFont* thelabel, bool pointercrate) {
    static nlohmann::json childJson;

    self->retain();

    std::string resultat;
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    std::string lvlname = level->levelName;
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

    if (curl) {
        std::string url = pointercrate ? "https://pointercrate.com" : "https://challengelist.gd";
        curl_easy_setopt(curl, CURLOPT_URL, url + "/api/v2/demons/listed/?name=" + std::string(lvlname));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultat);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (CURLE_OK != res) {
            std::string errormsg = "Error";
            thelabel->setString(errormsg.c_str());
            std::cerr << "CURL error: " << res << '\n';
            curl_global_cleanup();
            return;
        }
    }
    curl_global_cleanup();
    std::cout << resultat << "\n\n";
    childJson = nlohmann::json::parse(resultat);

    self->autorelease();

    if (childJson[0].contains("position")) {
        if (!pointercrate && level->userName != childJson[0]["publisher"]["name"]) {
            thelabel->setString("N/A");
            infoButton(self, thelabel);
            cachedPositions.insert({ level->levelID, -1 });
            return;
        }
        int position = childJson[0]["position"];
        std::string label = std::string(std::to_string(position));
        thelabel->setString(label.c_str());
        CCDictionary* pos = CCDictionary::create();
        pos->setObject(CCInteger::create(position), "get");
        pos->setObject(CCBool::create(pointercrate), "domain");
        createButton(self, thelabel, pos, pointercrate);
        cachedPositions.insert({ level->levelID, position });
    }
    else {
        thelabel->setString("N/A");
        infoButton(self, thelabel);
        cachedPositions.insert({ level->levelID, -1 });
    }

    return;
}

bool __fastcall LevelInfoLayer::hook(CCLayer* self, void*, gd::GJGameLevel* level) {
    bool result = init(self, level);

    if (level->ratingsSum != 50) return result;
    if (level->demon != 1 && level->levelLength >= 2) return result;

    int offset = (level->coins == 0) ? 17 : 4;

    auto director = CCDirector::sharedDirector();
    auto size = director->getWinSize();
    auto it = cachedPositions.find(level->levelID);

    bool pointercrate = (level->stars == 10) ? true : false;

    int yoffset = pointercrate ? 0 : 7.f;

    CCLabelBMFont* thelabel;
    thelabel = CCLabelBMFont::create(" ... ", "goldFont.fnt");
    
    thelabel->setPosition({ size.width / 2 - 100, size.height / 2 + offset + yoffset });
    thelabel->setScale(0.5f);

    if (!pointercrate) {
        thelabel->setFntFile("bigFont.fnt");
        thelabel->setScale(0.4f);
    }

    if (it != cachedPositions.end()) {
        if (cachedPositions[level->levelID] > -1) {
            int position = cachedPositions[level->levelID];
            std::string label = std::string(std::to_string(position));
            thelabel->setString(label.c_str());
            CCDictionary* pos = CCDictionary::create();
            pos->setObject(CCBool::create(pointercrate), "domain");
            pos->setObject(CCInteger::create(position), "get");
            createButton(self, thelabel, pos, pointercrate);
        }
        else {
            thelabel->setString("N/A");      
            infoButton(self, thelabel);
        }
    }
    else {
        std::thread worker1(onHttpRequestCompleted, self, level, thelabel, pointercrate);
        worker1.detach();
    }

    self->addChild(thelabel);


    return result;
}