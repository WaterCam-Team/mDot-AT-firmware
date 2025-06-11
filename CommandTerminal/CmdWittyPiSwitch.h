
#ifndef __CMDWITTYPISWITCH_H__
#define __CMDWITTYPISWITCH_H__

#include "Command.h"

class CommandTerminal;

class CmdWittyPiSwitch : public Command {

public:

    CmdWittyPiSwitch();
    virtual uint32_t action(const std::vector<std::string>& args);
    virtual bool verify(const std::vector<std::string>& args);
    
private:   
    
};

#endif // __CmdWittyPiSwitch_H__
