#pragma once
#include "libs/plog/Appenders/IAppender.h"
#include "libs/plog/WinApi.h"

namespace plog
{
    template<class Formatter>
    class DebugOutputAppender : public IAppender
    {
    public:
        virtual void write(const Record& record)
        {
            OutputDebugStringW(Formatter::format(record).c_str());
        }
    };
}
