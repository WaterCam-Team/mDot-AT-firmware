
#ifndef __CMDWITTYPISWITCH_H__
#define __CMDWITTYPISWITCH_H__

#include "Command.h"

class CommandTerminal;

class CmdWittyPiSwitch : public Command {

public:

    CmdWittyPiSwitch();
    static uint32_t action(std::vector<std::string> args);
    static bool verify(std::vector<std::string> args);
    
private:   
    
};

#endif // __CmdWittyPiSwitch_H__
