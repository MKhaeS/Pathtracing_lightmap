#include "Time.h"

Time::Time ()
{    
    QueryPerformanceFrequency (reinterpret_cast<LARGE_INTEGER*>(&m_Frequency_));
    __int64 cur;
    QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*>(&cur));    
    this->Restart ();
    this->m_Previous_ = cur;
}

int Time::Tick ()
{
    __int64 cur;
    QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*>(&cur));
    //delta is supposed to be small value
    int delta           = 1e6 * static_cast<float>(cur - this->m_Previous_) / m_Frequency_;
    this->m_Previous_   = cur;
    return delta;
}

__int64 Time::GetCurrent ()
{
    if (m_bEnabled_)
    {
        __int64 cur;
        QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*>(&cur));
        return cur;
    }
    else
    {
        return this->m_Current_;
    }
}

void Time::Restart ()
{
    this->m_bEnabled_ = true;
    this->m_Previous_ = 0;
    this->m_Current_  = 0;
}

void Time::Stop ()
{
    __int64 cur;
    QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*>(&cur));
    this->m_Current_ = cur;
    this->m_bEnabled_ = false;
}
