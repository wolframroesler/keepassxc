/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <QCoreApplication>
#include <QFile>
#include <QSaveFile>
#include <QStringList>
#include <QTextStream>

#include "core/Database.h"
#include "crypto/Crypto.h"
#include "format/KeePass2Reader.h"
#include "format/KeePass2Writer.h"
#include "keys/CompositeKey.h"
#include "keys/FileKey.h"
#include "keys/PasswordKey.h"


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (app.arguments().size() != 3) {
        qCritical("Usage: merge-databases <kdbx file1> <kdbx file2>");
        return 1;
    }

    if (!Crypto::init()) {
        qFatal("Fatal error while testing the cryptographic functions:\n%s", qPrintable(Crypto::errorString()));
    }

    static QTextStream inputTextStream(stdin, QIODevice::ReadOnly);

    QString line1 = inputTextStream.readLine();
    CompositeKey key1;
    if (QFile::exists(line1)) {
        FileKey fileKey;
        fileKey.load(line1);
        key1.addKey(fileKey);
    }
    else {
        PasswordKey password;
        password.setPassword(line1);
        key1.addKey(password);
    }

    QString line2 = inputTextStream.readLine();
    CompositeKey key2;
    if (QFile::exists(line2)) {
        FileKey fileKey;
        fileKey.load(line2);
        key2.addKey(fileKey);
    }
    else {
        PasswordKey password;
        password.setPassword(line2);
        key2.addKey(password);
    }


    QString databaseFilename1 = app.arguments().at(1);
    QFile dbFile1(databaseFilename1);
    if (!dbFile1.exists()) {
        qCritical("File %s does not exist.", qPrintable(databaseFilename1));
        return 1;
    }
    if (!dbFile1.open(QIODevice::ReadOnly)) {
        qCritical("Unable to open file %s.", qPrintable(databaseFilename1));
        return 1;
    }

    KeePass2Reader reader1;
    Database* db1 = reader1.readDatabase(&dbFile1, key1);

    if (reader1.hasError()) {
        qCritical("Error while parsing the database:\n%s\n", qPrintable(reader1.errorString()));
        return 1;
    }


    QString databaseFilename2 = app.arguments().at(2);
    QFile dbFile2(databaseFilename2);
    if (!dbFile2.exists()) {
        qCritical("File %s does not exist.", qPrintable(databaseFilename2));
        return 1;
    }
    if (!dbFile2.open(QIODevice::ReadOnly)) {
        qCritical("Unable to open file %s.", qPrintable(databaseFilename2));
        return 1;
    }

    KeePass2Reader reader2;
    Database* db2 = reader2.readDatabase(&dbFile2, key2);

    if (reader2.hasError()) {
        qCritical("Error while parsing the database:\n%s\n", qPrintable(reader2.errorString()));
        return 1;
    }

    db1->merge(db2);

    QSaveFile saveFile(databaseFilename1);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qCritical("Unable to open file %s for writing.", qPrintable(databaseFilename1));
        return 1;
    }

    KeePass2Writer writer;
    writer.writeDatabase(&saveFile, db1);

    if (writer.hasError()) {
        qCritical("Error while updating the database:\n%s\n", qPrintable(writer.errorString()));
        return 1;
    }

    if (!saveFile.commit()) {
        qCritical("Error while updating the database:\n%s\n", qPrintable(writer.errorString()));
        return 0;
    }

    qDebug("Successfully merged the database files.\n");
    return 1;

}
