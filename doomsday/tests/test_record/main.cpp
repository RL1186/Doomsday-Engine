/*
 * The Doomsday Engine Project
 *
 * Copyright (c) 2009-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <de/TextApp>
#include <de/Record>
#include <de/Reader>
#include <de/Writer>
#include <de/TextValue>
#include <de/NumberValue>
#include <de/Variable>
#include <de/data/json.h>

#include <QDebug>
#include <QTextStream>

using namespace de;

int main(int argc, char **argv)
{
    try
    {
        TextApp app(argc, argv);
        app.initSubsystems(App::DisablePlugins);

        Record rec;

        LOG_MSG("Empty record:\n") << rec;

        rec.add(new Variable("hello", new TextValue("World!")));
        LOG_MSG("With one variable:\n") << rec;

        rec.add(new Variable("size", new NumberValue(1024)));
        LOG_MSG("With two variables:\n") << rec;

        LOG_MSG("Record as JSON:\n") << composeJSON(rec).constData();

        Record rec2;
        Block b;
        Writer(b) << rec;
        LOG_MSG("Serialized record to ") << b.size() << " bytes.";

        String str;
        QTextStream os(&str);
        for (duint i = 0; i < b.size(); ++i)
        {
            os << dint(b.data()[i]) << " ";
        }
        LOG_MSG(str);

        Reader(b) >> rec2;
        LOG_MSG("After being deserialized:\n") << rec2;

        Record before;
        before.addSubrecord("subrecord");
        before.subrecord("subrecord").set("value", true);
        DENG2_ASSERT(before.hasSubrecord("subrecord"));
        LOG_MSG("Before copying:\n") << before;

        Record copied = before;
        DENG2_ASSERT(copied.hasSubrecord("subrecord"));
        LOG_MSG("Copied:\n") << copied;

        LOG_MSG("...and as JSON:\n") << composeJSON(copied).constData();
    }
    catch (Error const &err)
    {
        qWarning() << err.asText();
    }

    qDebug() << "Exiting main()...";
    return 0;
}
