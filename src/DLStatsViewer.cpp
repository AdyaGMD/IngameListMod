#include "DLStatsViewer.h"

void DLStatsViewer::setup() {
    // Use this how you would usually create layers

    //this->m_pLayer->addChild(title);
    // ^^ this is to add to the layer
    setMouseEnabled(true);
};

DLStatsViewer* DLStatsViewer::create() {
    auto pRet = new DLStatsViewer();
    //if (pRet && pRet->init(380.0f, 250.0f)) {
    if (pRet && pRet->init(500.0f, 280.0f, "GJ_square04.png")) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
};