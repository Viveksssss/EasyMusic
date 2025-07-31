#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
class Settings
{
public:
    Settings();
    static void saveLastUsedDirectory(const QString &path);
    static QString loadLastUsedDirectory();
};

#endif // SETTINGS_H
