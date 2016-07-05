#ifndef FILEDUMPCALLBACKIFCE_H
#define FILEDUMPCALLBACKIFCE_H

#include <string>

class FileDumpCallbackIfce {
public:
    virtual void onFileDumpComplete() = 0;
};

#endif // FILEDUMPCALLBACKIFCE_H
