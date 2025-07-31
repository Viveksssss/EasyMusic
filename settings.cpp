#include "settings.h"
#include <QSettings>
#include <QDir>
Settings::Settings()
{

}

void Settings::saveLastUsedDirectory(const QString &path)
{
    QSettings settings("Vivek", "myMusic"); // 注册表路径：HKEY_CURRENT_USER\Software\YourCompany\YourApp
    settings.setValue("LastUsedDirectory", path);
}

QString Settings::loadLastUsedDirectory()
{
    static QSettings settings("Vivek","myMusic");
    QString paths = settings.value("LastUsedDirectory",QDir::homePath()).toString();
    return paths;
}
