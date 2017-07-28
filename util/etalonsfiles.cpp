#include "etalonsfiles.h"

#include <cstdio>
#include <set>
#include <QDirIterator>

using namespace std;

EtalonsFiles::EtalonsFiles()
{
    QString search_path = QDir::currentPath();
    search_path += "/etalons";
    fprintf( stderr, "\nEtalonsFiles::EtalonsFiles() use search_path '%s'\n", search_path.toUtf8().constData() );
    QDirIterator it( search_path, QDir::Dirs, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        string dirname = it.fileName().toUtf8().constData();
        if ( dirname != string(".") && dirname != string("..") ) {
            int freqMhz = 0;
            int res = sscanf(dirname.c_str(), "%d", &freqMhz );
            if ( res == 1 ) {
                std::vector<string> antfiles(4, string("nofile"));
                std::set<int> antset;
                //fprintf( stderr, "%d MHz '%s'\n", freqMhz, dirname.c_str() );
                QDirIterator itfiles( it.filePath(), QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);
                while (itfiles.hasNext()) {
                    itfiles.next();
                    QString fname = itfiles.fileName().toUtf8().constData();
                    bool ok = false;
                    int ant = fname.mid( fname.lastIndexOf( QString("ant") ) + 3, 1 ).toInt( &ok );
                    if ( ok && (ant >= 1) && (ant <= 4) ) {
                        antset.insert(ant);
                        antfiles[ant-1] = string(itfiles.filePath().toUtf8().constData());
                    }
                }
                if ( antset.size() == 4 ) {
                    db[ freqMhz ] = antfiles;
                }
            }
        }
    }
    print();
}

EtalonsFiles::~EtalonsFiles()
{

}

std::string EtalonsFiles::GetFileFor(double freq, int ant_idx)
{
    int freqMhz = round(freq/1000000.0);
    //fprintf( stderr, "EtalonsFiles::GetFileFor() looking for %d MHz\n", freqMhz );

    if ( db.size() == 0 ) {
        return string("etalons_db_is_null");
    }

    string* best_fname = &string( "nofilename" );
    int best_diff = 10000;
    int best_freq = 0;

    for ( auto& x : db ) {
        int diff = abs( x.first - freqMhz );
        if ( diff < best_diff ) {
            best_diff = diff;
            best_fname = &( x.second[ant_idx] );
            best_freq = x.first;
        }
    }
    //fprintf( stderr, "Using file for %d MHz, diff = %d MHz\n", best_freq, best_diff );
    return *best_fname;
}

void EtalonsFiles::print() const {
    for ( auto& x : db ) {
        fprintf( stderr, "Freq %d MHz:\n", x.first );
        for ( size_t i = 0; i < x.second.size(); i++ ) {
            fprintf( stderr, "ant%u: %s\n", i, x.second[i].c_str() );
        }
    }
}
