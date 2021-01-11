
#include "pch.h"
export module test;


export class WinErrorLogger2 : public Log
{
    virtual void crash_error(std::string message, std::string str) override
    {
        std::string msg = message + " at \n" + str + " \n Exit?";

        if (MessageBox(NULL, msg.c_str(), "ERROR", MB_YESNO) == IDNO)
        {
            if (Application::is_good())
                Application::get().shutdown();
        }
    }
};