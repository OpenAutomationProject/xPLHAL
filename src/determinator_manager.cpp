#include "determinator_manager.h"
#include "determinator.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

using std::cout;
using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::filesystem_error;
using boost::filesystem::is_regular_file;
using boost::filesystem::is_directory;
using boost::filesystem::directory_iterator;
using boost::filesystem::ifstream;
        
DeterminatorManager::DeterminatorManager(const std::string& determinatorDirectory)
:mDeterminatorDirectory(determinatorDirectory)
{
}

void DeterminatorManager::loadDeterminators()
{
    path p(mDeterminatorDirectory);
    try {
        if (exists(p)) {
            if (is_regular_file(p)) {
                cout << p << " is regular file\n";
            }
            else if (is_directory(p)) {
                cout << p << " directory containing:\n";

                for (auto item = directory_iterator(p); item != directory_iterator(); ++item) {
                    ifstream determinatorFile(*item);
                    std::string filename = item->path().string();
                    //storeXmlDeterminator("guid", determinatorFile.read());
                    cout << "diritem: " << *item << "\n";
                    DeterminatorXmlParser parser(filename);
                    Determinator d = parser.parse();
                }
            }
            else {
                cout << p << " exists, but is neither a regular file nor a directory\n";
            }
        }
    }
    catch (const filesystem_error& ex) {

    }
}

std::string DeterminatorManager::getXmlDeterminator(const std::string& guid) const
{
}

void DeterminatorManager::storeXmlDeterminator(const std::string& guid, const std::string& xmlDeterminator)
{
}

