
#ifndef __CMDHEARTBEAT_H__
#define __CMDHEARTBEAT_H__

#include "Command.h"

class CmdHeartbeat : public Command {

public:

    CmdHeartbeat();
    virtual uint32_t action(const std::vector<std::string>& args);
    virtual bool verify(const std::vector<std::string>& args);
    
private:
    

};

#endif // __CMDHEARTBEAT_H__
