#pragma once

#include <vector>
#include <Windows.h>
#include <algorithm>
#include <functional>
#include <memory>

class EventHandler
{
public:
    EventHandler () {
        handlerFuncs = std::make_unique <std::vector<std::function<void (WPARAM, LPARAM)>>>();
    }
private:
    
    std::unique_ptr<std::vector<std::function<void (WPARAM, LPARAM)>>> handlerFuncs;
    int m_nFunctions = 0;
public:
    void operator += (std::function<void (WPARAM, LPARAM)> f) {
        handlerFuncs->push_back (f);
        ++m_nFunctions;
    }
    void operator ()(WPARAM wParam, LPARAM lParam) const {
        for (auto hF : *handlerFuncs) {
            hF (wParam, lParam);
        }
    }
    int GetNumOfFunctions () {
        return m_nFunctions;
    }

};
