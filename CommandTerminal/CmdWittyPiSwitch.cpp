#include "CmdWittyPiSwitch.h"

CmdWittyPiSwitch::CmdWittyPiSwitch() :
 Command("WittyPi Wake Signal", "AT+WPS", "Set GPIO pin high", "wittypi")
{
    _queryable = true;
}

uint32_t CmdWittyPiSwitch::action(const std::vector<std::string>& args)
{
    DigitalOut pin(PB_1);
    if (args.size() == 1)
    {
        CommandTerminal::Serial()->writef("Set Pin HIGH");
        pin = 1;
        ThisThread::sleep_for(2000ms);
        pin = 0;
    }

    return 0;
}

bool CmdWittyPiSwitch::verify(const std::vector<std::string>& args)
{
    if (args.size() == 1)
        return true;

    CommandTerminal::setErrorMessage("Invalid arguments");
    return false;
}