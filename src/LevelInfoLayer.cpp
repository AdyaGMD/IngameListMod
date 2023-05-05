#define CURL_STATICLIB

#include "pch.h"
#include "LevelInfoLayer.h"
#include "curl/curl.h"
#include <thread>
#include "stdio.h"
#include "json.hpp"
#include <windows.h>
#include <shellapi.h>

std::unordered_map<std::string, int> cachedPositions;
bool requestfinished = false;

static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

void createButton(CCLayer* self, CCLabelBMFont* thelabel, CCDictionary* pos) {
    CCPoint position = { thelabel->getPositionX() + 8.f, thelabel->getPositionY() };
    auto button = gd::CCMenuItemSpriteExtra::create(thelabel, self, menu_selector(LevelInfoLayer::openLink));
    button->setUserObject(pos); // if this fails, try setUserData
    auto menu = CCMenu::create(); // To make the button "clickable"
    CCSprite* trophy = CCSprite::createWithSpriteFrameName("rankIcon_top10_001.png");
    trophy->setScale(0.5f);
    trophy->setPosition({-10.f , 5.f});
    menu->addChild(button);
    menu->setPosition(position);
    button->addChild(trophy);
    self->addChild(menu);
}

// could also convert these into the class, so you do not have to pass these variables around
void onHttpRequestCompleted(CCLayer* self, gd::GJGameLevel* level, CCLabelBMFont* thelabel) {
    static nlohmann::json childJson;

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
        curl_easy_setopt(curl, CURLOPT_URL, "https://pointercrate.com/api/v2/demons/listed?name=" + std::string(lvlname));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultat);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (CURLE_OK != res) {
            std::string errormsg = "Err";
            thelabel->setString(errormsg.c_str());
            std::cerr << "CURL error: " << res << '\n';
            curl_global_cleanup();
            return;
        }
    }
    curl_global_cleanup();
    std::cout << resultat << "\n\n";

    childJson = nlohmann::json::parse(resultat);

    if (childJson[0].contains("position")) {
        int position = childJson[0]["position"];
        std::string label = std::string(std::to_string(position));
        thelabel->setString(label.c_str());
        CCDictionary* pos = CCDictionary::create();
        pos->setObject(CCInteger::create(position), "get");
        createButton(self, thelabel, pos);
        cachedPositions.insert({ level->levelName, position });
    }
    else {
        thelabel->setString("N/A");
        cachedPositions.insert({ level->levelName, -1 });
    }

    return;
}

void LevelInfoLayer::openLink(CCObject* ret) {
    CCDictionary* dict = static_cast<CCDictionary*>(static_cast<CCNode*>(ret)->getUserObject());
    CCInteger* position = reinterpret_cast<CCInteger*>(dict->objectForKey("get")); //could probably also use reinterpret_cast
    std::string url = "https://pointercrate.com/demonlist/" + std::string(std::to_string(position->getValue()));
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
}

bool __fastcall LevelInfoLayer::hook(CCLayer* self, void*, gd::GJGameLevel* level) {
    bool result = init(self, level);

    if (level->demonDifficulty != 6) return result;

    CCLabelBMFont* thelabel;
    thelabel = CCLabelBMFont::create("...", "goldFont.fnt");

    int offset = (level->coins == 0) ? 17 : 4;

    auto director = CCDirector::sharedDirector();
    auto size = director->getWinSize();
    auto it = cachedPositions.find(level->levelName);

    thelabel->setPosition({ size.width / 2 - 100, size.height / 2 + offset });
    thelabel->setScale(0.5f);

    if (it != cachedPositions.end()) {
        if (cachedPositions[level->levelName] > -1) {
            int position = cachedPositions[level->levelName];
            std::string label = std::string(std::to_string(position));
            thelabel->setString(label.c_str());
            CCDictionary* pos = CCDictionary::create();
            pos->setObject(CCInteger::create(position), "get");
            createButton(self, thelabel, pos); // probably a bad idea passing it
        }
        else {
            thelabel->setString("N/A");        
        }
    }
    else {
        std::thread worker1(onHttpRequestCompleted, self, level, thelabel);
        worker1.detach();
    }

    self->addChild(thelabel);


    return result;
}