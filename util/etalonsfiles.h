#ifndef ETALONSFILES_H
#define ETALONSFILES_H

#include <string>
#include <vector>
#include <map>

class EtalonsFiles
{
public:
    EtalonsFiles();
    ~EtalonsFiles();

    std::string GetFileFor( double freq, int ant_idx );
    void print() const;
private:
    std::map< int, std::vector<std::string> > db;
};

#endif // ETALONSFILES_H
