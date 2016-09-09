#ifndef _TIMER_H_
#define _TIMER_H_


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"


class Time {
public:
                    Time ();
    int             Tick ();
    __int64         GetCurrent ();
    void            Restart ();
    void            Stop ();

private:
    bool            m_bEnabled_  = false;
    __int64         m_Current_   = 0;
    __int64         m_Previous_  = 0;
    __int64         m_Frequency_;

    
};


#endif