/*
 * The Doomsday Engine Project -- libdeng2
 *
 * Copyright © 2009-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small> 
 */

#ifndef LIBDENG2_FILESYSTEM_H
#define LIBDENG2_FILESYSTEM_H

#include "../libdeng2.h"
#include "../Folder"
#include "../System"

#include <QFlags>
#include <map>

/**
 * @defgroup fs File System
 *
 * The file system (de::FileSystem) governs a tree of files and folders, and
 * provides the means to access all data in libdeng2. It follows the metaphor
 * of a UNIX file system, where not all files are "regular" files, but instead
 * may represent non-file objects that still support serialization into byte
 * arrays or have a byte-stream input/output interface. This way it provides a
 * uniform interface to all public data that is compatible with network
 * communications, persistence, hierarchical organization and lookup, item
 * metadata (names, modification timestamps, custom key/values) and scripting.
 *
 * To facilitate efficient O(log n) searches over the entire file system,
 * de::FileSystem maintains an index of all files and folders by name. There is
 * additionally a separate index for each file type (e.g., de::ArchiveEntryFile).
 *
 * The file system has to be manually refreshed when the underlying data
 * changes. For instance, when new files are written to a folder on the hard
 * drive, one must call de::FileSystem::refresh() for the changes to be reflected
 * in the de::FileSystem index and tree.
 *
 * ZIP (PK3) archives are visible in the libdeng2 file system as Folder and
 * File instances just like regular native files are. This allows one to deploy
 * a large collection of resources as an archive and treat it at runtime just
 * like a tree of native files. Files within archives can be read and written
 * just like native files, and the containing archives will be updated as
 * needed.
 * @see de::ArchiveEntryFile, de::ArchiveFeed, and de::FileSystem::interpret()
 */

namespace de {

namespace internal {
    template <typename Type>
    inline bool cannotCastFileTo(File *file) {
        return dynamic_cast<Type *>(file) == NULL;
    }
}

/**
 * The file system maintains a tree of files and folders. It provides a way
 * to quickly and efficiently locate files anywhere in the tree. It also
 * maintains semantic information about the structure and content of the
 * file tree, allowing others to know how to treat the files and folders.
 *
 * In practice, the file system consists of a tree of File and Folder
 * instances. These instances are generated by the Feed objects attached to
 * the folders. For instance, a DirectoryFeed will generate the appropriate
 * File and Folder instances for a directory in the native file system.
 *
 * Wildcard searches are discouraged as implementing them is potentially
 * inefficient. Instead, suitable indices should be built beforehand if
 * there is a need to look up lots of files matching a specific criteria
 * from unknown locations in the tree.
 *
 * The file system can be repopulated at any time to resynchronize it with
 * the source data. Repopulation is nondestructive as long as the source
 * data has not changed. Repopulation is needed for instance when native
 * files get deleted in the directory a folder is feeding on. The feeds are
 * responsible for deciding when instances get out-of-date and need to be
 * deleted (pruning). Pruning occurs when a folder that is already
 * populated with files is repopulated.
 *
 * @ingroup fs
 */
class DENG2_PUBLIC FileSystem : public System
{
public:
    /// No index is found for the specified type. @ingroup errors
    DENG2_ERROR(UnknownTypeError);

    /// No files found. @ingroup errors
    DENG2_ERROR(NotFoundError);

    /// More than one file found and there is not enough information to choose
    /// between them. @ingroup errors
    DENG2_ERROR(AmbiguousError);

    typedef std::multimap<String, File *> Index;
    typedef std::pair<Index::iterator, Index::iterator> IndexRange;
    typedef std::pair<Index::const_iterator, Index::const_iterator> ConstIndexRange;
    typedef std::pair<String, File *> IndexEntry;
    typedef std::list<File *> FoundFiles;

public:
    /**
     * Constructs a new file system. The file system needs to be manually
     * refreshed; initially it is empty.
     */
    FileSystem();

    void printIndex();

    Folder &root();

    /**
     * Refresh the file system. Populates all folders with files from the feeds.
     */
    void refresh();

    enum FolderCreationBehavior {
        DontInheritFeeds   = 0,     ///< Subfolder will not have any feeds created for them.
        InheritPrimaryFeed = 0x1,   ///< Subfolder will inherit the primary (first) feed of its parent.
        InheritAllFeeds    = 0x2,   ///< Subfolder will inherit all feeds of its parent.
        PopulateNewFolder  = 0x4,   ///< Populate new folder automatically.

        InheritPrimaryFeedAndPopulate = InheritPrimaryFeed | PopulateNewFolder
    };
    Q_DECLARE_FLAGS(FolderCreationBehaviors, FolderCreationBehavior)

    /**
     * Retrieves a folder in the file system. The folder gets created if it
     * does not exist. Any missing parent folders will also be created.
     *
     * @param path      Path of the folder. Relative to the root folder.
     * @param behavior  What to do with the new folder: automatically attach feeds and
     *                  maybe also populate it automatically.
     *
     * @return  Folder at @a path.
     */
    Folder &makeFolder(String const &path,
                       FolderCreationBehaviors behavior = InheritPrimaryFeedAndPopulate);

    /**
     * Retrieves a folder in the file system and replaces all of its existing
     * feeds with the specified feed. The folder gets created if it does not
     * exist. If it does exist, the folder will be cleared so that any existing
     * contents won't be orphaned due to the previous feeds going away.
     *
     * Any missing parent folders will also be created.
     *
     * @param path      Path of the folder. Relative to the root folder.
     * @param feed      Primary feed for the folder (other feeds removed).
     * @param behavior  Behavior for creating folders (including missing parents).
     * @param populationBehavior  How to populate the returned folder (if population requested).
     *
     * @return  Folder at @a path.
     */
    Folder &makeFolderWithFeed(String const &path, Feed *feed,
                               Folder::PopulationBehavior populationBehavior = Folder::PopulateFullTree,
                               FolderCreationBehaviors behavior = InheritPrimaryFeedAndPopulate);

    /**
     * Finds all files matching a full or partial path. The search is done
     * using the file system's index; no recursive descent into folders is
     * done.
     *
     * @param path   Path or file name to look for.
     * @param found  Set of files that match the result.
     *
     * @return  Number of files found.
     */
    int findAll(String const &path, FoundFiles &found) const;

    /**
     * Finds a single file matching a full or partial path. The search is
     * done using the file system's index; no recursive descent into
     * folders is done.
     *
     * @param path   Path or file name to look for.
     *
     * @return  The found file.
     */
    File &find(String const &path) const;

    /**
     * Finds a file of a specific type. The search is done using the file
     * system's index; no recursive descent into folders is done. Only
     * files that can be represented as @a Type are included in the
     * results.
     *
     * @param path  Full/partial path or file name to look for.
     *
     * @see indexFor() returns the full index for a particular type of file
     * for manual searches.
     */
    template <typename Type>
    Type &find(String const &path) const {
        FoundFiles found;
        findAll(path, found);
        // Filter out the wrong types.
        found.remove_if(internal::cannotCastFileTo<Type>);
        if(found.size() > 1) {
            /// @throw AmbiguousError  More than one file matches the conditions.
            throw AmbiguousError("FS::find", "More than one file found matching '" + path + "'");
        }
        if(found.empty()) {
            /// @throw NotFoundError  No files found matching the condition.
            throw NotFoundError("FS::find", "No files found matching '" + path + "'");
        }
        return *dynamic_cast<Type *>(found.front());
    }

    /**
     * Creates an interpreter for the data in a file.
     *
     * @param sourceData  File with the source data. While interpreting,
     *                    ownership of the file is given to de::FileSystem.
     *
     * @return If the format of the source data was recognized, returns a new
     * File (or Folder) that can be used for accessing the data. Caller gets
     * ownership of the returned instance. Ownership of the @a sourceData will
     * be transferred to the returned instance. If the format was not
     * recognized, @a sourceData is returned as-is and ownership is returned to
     * the caller.
     */
    File *interpret(File *sourceData);

    /**
     * Provides access to the main index of the file system. This can be
     * used for efficiently looking up files based on name.
     *
     * @note The file names are indexed in lower case.
     */
    Index const &nameIndex() const;

    /**
     * Retrieves the index of files of a particular type.
     *
     * @param typeIdentifier  Type identifier to look for. Use the DENG2_TYPE_NAME() macro.
     *
     * @return A subset of the main index containing only the entries of
     * the given type.
     *
     * For example, to look up the index for NativeFile instances:
     * @code
     * FS::Index &nativeFileIndex = App::fileSystem().indexFor(DENG2_TYPE_NAME(NativeFile));
     * @endcode
     */
    Index const &indexFor(String const &typeIdentifier) const;

    /**
     * Adds a file to the main index.
     *
     * @param file  File to index.
     */
    void index(File &file);

    /**
     * Removes a file from the main index.
     *
     * @param file  File to remove from the index.
     */
    void deindex(File &file);

    enum CopyBehavior
    {
        PlainFileCopy          = 0,
        ReinterpretDestination = 0x1,
        PopulateDestination    = 0x2,

        DefaultCopyBehavior = ReinterpretDestination | PopulateDestination
    };
    Q_DECLARE_FLAGS(CopyBehaviors, CopyBehavior)

    /**
     * Makes a copy of a file by streaming the bytes of the source path to the
     * destination path.
     *
     * @param sourcePath       Source path.
     * @param destinationPath  Destination path.
     */
    File &copySerialized(String const &sourcePath, String const &destinationPath,
                         CopyBehaviors behavior = DefaultCopyBehavior);

    void timeChanged(Clock const &);

private:
    DENG2_PRIVATE(d)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FileSystem::FolderCreationBehaviors)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileSystem::CopyBehaviors)

// Alias.
typedef FileSystem FS;

} // namespace de

#endif // LIBDENG2_FILESYSTEM_H
