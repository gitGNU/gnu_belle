/* Copyright (C) 2012, 2013 Carlos Pais 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "engine.h"

#include <QStringList>
#include <QDir>
#include <QObject>
#include <QMessageBox>

static QString mPath = "";
static QString mBrowserPath = "";

bool Engine::isValidPath(const QString & path)
{
    if (path.isEmpty() || path.isNull())
        return false;

    QDir dir(path);

    QStringList engineComponents;
    engineComponents << "index.html" << "display.js" << "actions.js" << "objects.js" << "utils.js"
                     << "buzz.js" << "style.css" << "events.js" << "jquery.js" << "consolelog.js" << "json-fallback.js";

    //check if all necessary files seem to be present
    QStringList missingFiles;

    foreach(const QString& file, engineComponents)
        if (! dir.exists(file))
            missingFiles << file;

    if (! missingFiles.isEmpty())
        return false;
    return true;
}

void Engine::setPath(const QString & path)
{
    mPath = path;
}

QString Engine::path()
{
    return mPath;
}

bool Engine::isValid()
{
    return isValidPath(mPath);
}

void Engine::guessPath()
{
    QString enginePath("Engine");
    QDir currDir = QDir::current();

    if (currDir.cd(enginePath) || currDir.cd(enginePath.toLower()))
        mPath = currDir.absolutePath();

    if (! isValidPath(mPath)) {
        currDir.cdUp();
        if (currDir.cd(enginePath) || currDir.cd(enginePath.toLower()))
            mPath = currDir.absolutePath();
    }
}

QString Engine::browserPath()
{
    return mBrowserPath;
}

void Engine::setBrowserPath(const QString& path)
{
    mBrowserPath = path;
}
