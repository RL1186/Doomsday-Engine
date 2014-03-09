/** @file savedsessionrepository.cpp  Saved (game) session repository.
 *
 * @authors Copyright © 2014 Daniel Swanson <danij@dengine.net>
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

#include "de/game/SavedSessionRepository"

#include "de/App"
#include "de/game/Game"
#include "de/game/IMapStateReader"
#include "de/Log"
#include "de/NativePath"
#include "de/PackageFolder"
#include "de/ZipArchive"

#include <QList>
#include <map>

namespace de {
namespace game {

DENG2_PIMPL(SavedSessionRepository)
, DENG2_OBSERVES(App, GameUnload)
//, DENG2_OBSERVES(App, GameChange)
{
    Folder *folder; ///< Root of the saved session repository file structure.

    typedef std::map<String, SavedSession *> Sessions;
    Sessions sessions;

    struct ReaderInfo {
        //GameStateRecognizeFunc recognize;
        MapStateReaderMakeFunc newReader;

        ReaderInfo(/*GameStateRecognizeFunc recognize,*/ MapStateReaderMakeFunc newReader)
            : /*recognize(recognize),*/ newReader(newReader)
        {
            DENG2_ASSERT(/*recognize != 0 &&*/ newReader != 0);
        }
    };
    typedef QList<ReaderInfo> ReaderInfos;
    ReaderInfos readers;

    Instance(Public *i)
        : Base(i)
        , folder(0)
    {
        App::app().audienceForGameUnload() += this;
        //App::app().audienceForGameChange() += this;
    }

    ~Instance()
    {
        App::app().audienceForGameUnload() += this;
        //App::app().audienceForGameChange() += this;

        clearSessions();
    }

    void clearSessions()
    {
        qDebug() << "Clearing saved sessions in the repository";
        DENG2_FOR_EACH(Sessions, i, sessions) { delete i->second; }
        sessions.clear();
    }

    bool recognize(PackageFolder &pack, SessionMetadata &metadata) const
    {
        if(!pack.has("Info")) return false;

        File &file = pack.locate<File>("Info");
        Block raw;
        file >> raw;

        metadata.parse(String::fromUtf8(raw));

        int const saveVersion = metadata["version"].value().asNumber();
        if(saveVersion > 14) // Future version?
        {
            return false;
        }
        return true;
    }

    ReaderInfo const *readSessionMetadata(SavedSession &session) const
    {
        try
        {
            Path filePath = self.folder().path() / session.fileName();
            File &sourceData = App::fileSystem().find<File>(filePath);
            if(ZipArchive::recognize(sourceData))
            {
                try
                {
                    // It is a ZIP archive: we will represent it as a folder.
                    QScopedPointer<PackageFolder> package(new PackageFolder(sourceData, sourceData.name()));

                    // Archive opened successfully, give ownership of the source to the folder.
                    package->setSource(&sourceData);

                    foreach(ReaderInfo const &rdrInfo, readers)
                    {
                        // Attempt to recognize the game state and deserialize the metadata.
                        QScopedPointer<SessionMetadata> metadata(new SessionMetadata);
                        if(recognize(*package, *metadata))
                        {
                            session.replaceMetadata(metadata.take());
                            return &rdrInfo;
                        }
                    }
                }
                catch(Archive::FormatError const &)
                {
                    LOG_RES_WARNING("Archive in %s is invalid") << sourceData.description();
                }
                catch(IByteArray::OffsetError const &)
                {
                    LOG_RES_WARNING("Archive in %s is truncated") << sourceData.description();
                }
                catch(IIStream::InputError const &)
                {
                    LOG_RES_WARNING("%s cannot be read") << sourceData.description();
                }
            }
        }
        catch(FileSystem::NotFoundError const &)
        {}  // Ignore this error.

        return 0; // Unrecognized
    }

    void aboutToUnloadGame(game::Game const & /*gameBeingUnloaded*/)
    {
        // Remove the saved sessions (not serialized state files).
        /// @note Once the game state file is read with an engine-side file reader, clearing
        /// the sessions at this time is not necessary.
        clearSessions();

        // Clear the registered game state readers too.
        readers.clear();
    }
};

SavedSessionRepository::SavedSessionRepository() : d(new Instance(this))
{}

Folder const &SavedSessionRepository::folder() const
{
    DENG2_ASSERT(d->folder != 0);
    return *d->folder;
}

Folder &SavedSessionRepository::folder()
{
    DENG2_ASSERT(d->folder != 0);
    return *d->folder;
}

void SavedSessionRepository::setLocation(Path const &location)
{
    qDebug() << "(Re)Initializing saved session repository file structure at" << location.asText();

    // Clear the saved session db, we're starting over.
    d->clearSessions();

    if(!location.isEmpty())
    {
        // Initialize the file structure.
        try
        {
            d->folder = &App::fileSystem().makeFolder(location);
            d->folder->populate(Folder::PopulateOnlyThisFolder);
            /// @todo attach a feed to this folder.
            /// @todo scan the indexed files and populate the saved session db.
            return;
        }
        catch(Error const &)
        {}
    }

    LOG_RES_ERROR("\"%s\" could not be accessed. Perhaps it could not be created (insufficient permissions?)."
                  " Saving will not be possible.") << NativePath(location).pretty();
}

bool SavedSessionRepository::contains(String fileName) const
{
    return d->sessions.find(fileName) != d->sessions.end();
}

void SavedSessionRepository::add(String fileName, SavedSession *session)
{
    // Ensure the session identifier is unique.
    Instance::Sessions::iterator existing = d->sessions.find(fileName);
    if(existing != d->sessions.end())
    {
        if(existing->second != session)
        {
            delete existing->second;
            existing->second = session;
        }
    }
    else
    {
        d->sessions[fileName] = session;
    }
}

SavedSession &SavedSessionRepository::session(String fileName) const
{
    Instance::Sessions::iterator found = d->sessions.find(fileName);
    if(found != d->sessions.end())
    {
        DENG2_ASSERT(found->second != 0);
        return *found->second;
    }
    /// @throw MissingSessionError An unknown session was referenced.
    throw MissingSessionError("SavedSessionRepository::session", "Unknown session '" + fileName + "'");
}

void SavedSessionRepository::declareReader(/*GameStateRecognizeFunc recognizer,*/ MapStateReaderMakeFunc maker)
{
    d->readers.append(Instance::ReaderInfo(/*recognizer,*/ maker));
}

bool SavedSessionRepository::recognize(SavedSession &session) const
{
    return d->readSessionMetadata(session) != 0;
}

IMapStateReader *SavedSessionRepository::recognizeAndMakeReader(SavedSession &session) const
{
    if(Instance::ReaderInfo const *rdrInfo = d->readSessionMetadata(session))
    {
        return rdrInfo->newReader();
    }
    return 0; // Unrecognized
}

} // namespace game
} // namespace de
