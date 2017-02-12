/** @file archivefeed.cpp Archive Feed.
 *
 * @author Copyright © 2009-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @author Copyright © 2013 Daniel Swanson <danij@dengine.net>
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

#include "de/ArchiveFeed"
#include "de/ArchiveEntryFile"
#include "de/ArchiveFolder"
#include "de/ByteArrayFile"
#include "de/ZipArchive"
#include "de/Writer"
#include "de/Folder"
#include "de/FS"
#include "de/LogBuffer"

namespace de {

DENG2_PIMPL(ArchiveFeed)
, DENG2_OBSERVES(File, Deletion)
{
    /// File where the archive is stored (in a serialized format).
    File *file;

    /// The archive can be physically stored here, as Archive doesn't make a
    /// copy of the buffer.
    Block serializedArchive;

    Archive *arch;

    /// Mount point within the archive for this feed.
    String basePath;

    /// The feed whose archive this feed is using.
    ArchiveFeed *parentFeed;

    bool allowWrite;

    /// All the entries of this archive that are populated as files.
    /// Subfeeds use the parent's entry table.
    typedef LockableT<PointerSetT<ArchiveEntryFile>> EntryTable;
    EntryTable entries;

    Impl(Public *feed, File &f)
        : Base(feed)
        , file(&f)
        , arch(0)
        , parentFeed(0)
        , allowWrite(f.mode().testFlag(File::Write)) // write access depends on file
    {
        // If the file happens to be a byte array file, we can use it
        // directly to store the Archive.
        if (IByteArray *bytes = f.maybeAs<IByteArray>())
        {
            LOG_RES_XVERBOSE("Source %s is a byte array", f.description());

            arch = new ZipArchive(*bytes);
        }
        else
        {
            LOG_RES_XVERBOSE("Source %s is a stream", f.description());

            // The file is just a stream, so we can't rely on the file
            // acting as the physical storage location for Archive.
            f >> serializedArchive;
            arch = new ZipArchive(serializedArchive);
        }

        file->audienceForDeletion() += this;
    }

    Impl(Public *feed, ArchiveFeed &parentFeed, String const &path)
        : Base(feed)
        , file(parentFeed.d->file)
        , arch(0)
        , basePath(path)
        , parentFeed(&parentFeed)
        , allowWrite(isWriteAllowed())
    {
        file->audienceForDeletion() += this;
    }

    ~Impl()
    {
        if (arch)
        {
            writeIfModified();
            delete arch;
        }
    }

    bool isWriteAllowed() const
    {
        if (parentFeed)
        {
            return parentFeed->d->isWriteAllowed();
        }
        return allowWrite;
    }

    void writeIfModified()
    {
        if (!file || !arch || !file->mode().testFlag(File::Write))
        {
            return;
        }

        // If modified, the archive is written back to the file.
        if (arch->modified())
        {
            LOG_RES_MSG("Updating archive in ") << file->description();

            // Make sure we have either a compressed or uncompressed version of
            // each entry in memory before destroying the source file.
            arch->cache();

            file->clear();
            Writer(*file) << *arch;
            file->flush();
        }
        else
        {
            LOG_RES_VERBOSE("Not updating archive in %s (not changed)") << file->description();
        }
    }

    void fileBeingDeleted(File const &deleted)
    {
        if (file == &deleted)
        {
            // Ensure that changes are saved and detach from the file.
            writeIfModified();
            file = 0;
        }
        else
        {
            auto &table = entryTable();
            DENG2_GUARD(table);
            table.value.remove(&deleted.as<ArchiveEntryFile>());
        }
    }

    Archive &archive()
    {
        if (parentFeed)
        {
            return parentFeed->archive();
        }
        return *arch;
    }

    EntryTable &entryTable()
    {
        return parentFeed? parentFeed->d->entries : entries;
    }

    void addToEntryTable(ArchiveEntryFile *file)
    {
        auto &table = entryTable();
        DENG2_GUARD(table);
        table.value.insert(file);
        file->audienceForDeletion() += this;
    }

    PopulatedFiles populate(Folder const &folder)
    {
        PopulatedFiles populated;

        // Get a list of the files in this directory.
        Archive::Names names;
        archive().listFiles(names, basePath);

        DENG2_FOR_EACH(Archive::Names, i, names)
        {
            if (folder.has(*i))
            {
                // Already has an entry for this, skip it (wasn't pruned so it's OK).
                continue;
            }

            String entry = basePath / *i;

            std::unique_ptr<ArchiveEntryFile> archFile(new ArchiveEntryFile(*i, archive(), entry));
            addToEntryTable(archFile.get());

            // Write access is inherited from the main source file.
            if (allowWrite) archFile->setMode(File::Write);

            // Use the status of the entry within the archive.
            archFile->setStatus(archive().entryStatus(entry));

            // Interpret the entry contents.
            File *f = folder.fileSystem().interpret(archFile.release());

            // We will decide on pruning this.
            f->setOriginFeed(thisPublic);

            populated << f;
        }

        // Also create subfolders by inheriting from this feed. They will get
        // populated later.
        archive().listFolders(names, basePath);
        for (Archive::Names::iterator i = names.begin(); i != names.end(); ++i)
        {
            folder.fileSystem().makeFolder(folder.path() / *i, FS::InheritPrimaryFeed);
        }
        return populated;
    }
};

ArchiveFeed::ArchiveFeed(File &archiveFile)
    : d(new Impl(this, archiveFile))
{}

ArchiveFeed::ArchiveFeed(ArchiveFeed &parentFeed, String const &basePath)
    : d(new Impl(this, parentFeed, basePath))
{}

ArchiveFeed::~ArchiveFeed()
{
    LOG_AS("~ArchiveFeed");
    d.reset();
}

String ArchiveFeed::description() const
{
    return "archive in " + (d->file? d->file->description() : "(deleted file)");
}

Feed::PopulatedFiles ArchiveFeed::populate(Folder const &folder)
{
    LOG_AS("ArchiveFeed::populate");

    return d->populate(folder);
}

bool ArchiveFeed::prune(File &file) const
{
    LOG_AS("ArchiveFeed::prune");

    ArchiveEntryFile *entryFile = file.maybeAs<ArchiveEntryFile>();
    if (entryFile && &entryFile->archive() == &archive())
    {
        if (!archive().hasEntry(entryFile->entryPath()))
        {
            LOG_RES_VERBOSE("%s removed from archive") << file.description();
            return true; // It's gone...
        }

        // Prune based on entry status.
        if (archive().entryStatus(entryFile->entryPath()).modifiedAt !=
           file.status().modifiedAt)
        {
            LOG_RES_XVERBOSE("%s has been modified (arch:%s != file:%s)",
                             file.description()
                             << archive().entryStatus(entryFile->entryPath()).modifiedAt.asText()
                             << file.status().modifiedAt.asText());
            return true;
        }
    }
    return false;
}

File *ArchiveFeed::newFile(String const &name)
{
    String newEntry = d->basePath / name;
    if (archive().hasEntry(newEntry))
    {
        /// @throw AlreadyExistsError  The entry @a name already exists in the archive.
        throw AlreadyExistsError("ArchiveFeed::newFile", name + ": already exists");
    }
    // Add an empty entry.
    archive().add(newEntry, Block());
    auto *file = new ArchiveEntryFile(name, archive(), newEntry);
    d->addToEntryTable(file);
    file->setOriginFeed(this);
    return file;
}

void ArchiveFeed::removeFile(String const &name)
{
    archive().remove(d->basePath / name);
}

Feed *ArchiveFeed::newSubFeed(String const &name)
{
    return new ArchiveFeed(*this, d->basePath / name);
}

Archive &ArchiveFeed::archive()
{
    return d->archive();
}

Archive const &ArchiveFeed::archive() const
{
    return d->archive();
}

String const &ArchiveFeed::basePath() const
{
    return d->basePath;
}

File const &ArchiveFeed::archiveSourceFile() const
{
    if (d->file)
    {
        return *d->file;
    }
    throw InvalidSourceError("ArchiveFeed::archiveSourceFile", "Archive source file is gone");
}

void ArchiveFeed::rewriteFile()
{
    if (d->parentFeed)
    {
        DENG2_ASSERT(!d->arch);
        d->parentFeed->rewriteFile();
    }
    else
    {
        DENG2_ASSERT(d->arch != 0);
        d->writeIfModified();
    }
}

void ArchiveFeed::uncache()
{
    auto &table = d->entryTable();
    DENG2_GUARD(table);
    for (auto *entry : table.value)
    {
        entry->uncache();
    }
}

void ArchiveFeed::uncacheAllEntries(StringList const &folderTypes) // static
{
    if (Folder::isPopulatingAsync()) return; // Never mind.

    for (String const &folderType : folderTypes)
    {
        foreach (File *file, FileSystem::get().indexFor(folderType).files())
        {
            if (auto *feed = file->as<Folder>().primaryFeedMaybeAs<ArchiveFeed>())
            {
                feed->uncache();
            }
        }
    }
}

} // namespace de
