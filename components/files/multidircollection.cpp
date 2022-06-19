#include "multidircollection.hpp"

#include <filesystem>

#include <components/debug/debuglog.hpp>

namespace Files
{
    struct NameEqual
    {
        bool mStrict;

        NameEqual (bool strict) : mStrict (strict) {}

        bool operator() (const std::string& left, const std::string& right) const
        {
            if (mStrict)
                return left==right;
            return Misc::StringUtils::ciEqual(left, right);
        }
    };

    MultiDirCollection::MultiDirCollection(const Files::PathContainer& directories,
        const std::string& extension, bool foldCase)
    : mFiles (NameLess (!foldCase))
    {
        NameEqual equal (!foldCase);

        for (const auto & directory : directories)
        {
            if (!std::filesystem::is_directory(directory))
            {
                Log(Debug::Info) << "Skipping invalid directory: " << directory.string(); //TODO(Project579): This will probably break in windows with unicode paths
                continue;
            }

            for (std::filesystem::directory_iterator dirIter(directory);
                    dirIter != std::filesystem::directory_iterator(); ++dirIter)
            {
                std::filesystem::path path = *dirIter;

                if (!equal (extension, path.extension().string())) //TODO(Project579): let's hope unicode characters are never used in these extensions on windows or this will break
                    continue;

                std::string filename = path.filename().string(); //TODO(Project579): let's hope unicode characters are never used in these filenames on windows or this will break

                TIter result = mFiles.find (filename);

                if (result==mFiles.end())
                {
                    mFiles.insert (std::make_pair (filename, path));
                }
                else if (result->first==filename)
                {
                    mFiles[filename] = path;
                }
                else
                {
                    // handle case folding
                    mFiles.erase (result->first);
                    mFiles.insert (std::make_pair (filename, path));
                }
            }
        }
    }

    std::filesystem::path MultiDirCollection::getPath (const std::string& file) const
    {
        TIter iter = mFiles.find (file);

        if (iter==mFiles.end())
            throw std::runtime_error ("file " + file + " not found");

        return iter->second;
    }

    bool MultiDirCollection::doesExist (const std::string& file) const
    {
        return mFiles.find (file)!=mFiles.end();
    }

    MultiDirCollection::TIter MultiDirCollection::begin() const
    {
        return mFiles.begin();
    }

    MultiDirCollection::TIter MultiDirCollection::end() const
    {
        return mFiles.end();
    }
}
