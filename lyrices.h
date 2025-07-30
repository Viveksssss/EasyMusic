#ifndef LYRICES_H
#define LYRICES_H

#include <QString>
#include <QMap>
class Lyrics
{
public:
    Lyrics();
    Lyrics(QString lyricesPath);
    bool readLyricsFile(QString lyricsPath);
    bool analysisLyricsFile(QString line);
    QList<QString>getListLyricsText()const;
    QList<int> getListLyricsTime()const;
    QMap<int,QString>getMap()const;

    QString lyricsPath;
private:
    QList<QString>listLyricsText;
    QList<int>listLyricsTime;
    QMap<int,QString>lyricsMap;
};

#endif // LYRICES_H
