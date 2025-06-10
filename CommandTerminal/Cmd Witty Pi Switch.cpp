#include "CmdWittyPiSwitch.h"
#include "mbed.h"

CmdWittyPiSwitch::CmdWittyPiSwitch() :
 Command("WittyPi Wake Signal", "AT+WPWS", "Set GPIO pin high", "wittypi")
{
    _queryable = true;
}

uint32_t CmdWittyPiSwitch::action(std::vector<std::string> args)
{
    DigitalOut pin(PB_1);
    if (args.size() == 1)
    {
        //while(1) {
            CommandTerminal::Serial()->writef("Set Pin HIGH");
            pin = 1;
            wait(0.5);
            pin = 0;
            //wait(1);
        //}
    }

    return 0;
}

bool CmdWittyPiSwitch::verify(std::vector<std::string> args)
{
    if (args.size() == 1)
        return true;

    CommandTerminal::setErrorMessage("Invalid arguments");
    return false;
}