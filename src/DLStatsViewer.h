#pragma once
#include "pch.h"
#include "BrownAlertDelegate.h"

class DLStatsViewer : public BrownAlertDelegate {
protected:
    virtual void setup();
public:
    static DLStatsViewer* create();
};
