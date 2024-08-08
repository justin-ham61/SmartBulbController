#ifndef INIT_H
#define INIT_H

#include "config.h"
#include "KasaSmartPlug.h"
#include "./structs/device.h"

bool tryAdd(const char *alias, const char *ip, const char *type, KASAUtil kasaUtil){
    bool result = kasaUtil.CreateDevice(alias, ip, type);
}   

void addFromConfig(UserDevice devices[], KASAUtil kasaUtil){
    for(int i = 0; i < 4; i++){
        tryAdd(devices[i].name, devices[i].ip, devices[i].type, kasaUtil);
    }
}

#endif