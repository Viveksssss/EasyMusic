#include "lyrices.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
Lyrics::Lyrics(){

}
Lyrics::Lyrics(QString lyricesPath){
    this->lyricsPath = lyricesPath;
}
bool Lyrics::readLyricsFile(QString lyricsPath){
    this->lyricsMap.clear();
    QFile file(lyricsPath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        listLyricsText.clear();
        listLyricsTime.clear();
        return false;
    }
    QString line = "";
    while(!(line=file.readLine()).isEmpty()){
        qDebug() << line;
        analysisLyricsFile(line);
    }
    return true;
}
bool Lyrics::analysisLyricsFile(QString line){
    if(line == nullptr || line.isEmpty()){
        qDebug() << "empty line";
        return false;
    }
    QRegularExpression reg("\\[(\\d+)?:(\\d+)?(\\.\\d+)?\\](.*)?");
    int index = 0;
    QRegularExpressionMatch match;
    match = reg.match(line,index);
    if(match.hasMatch()){
        int totalTime ;
        totalTime = match.captured(1).toInt()*60000+match.captured(2).toInt()*1000;
        QString currentText = QString::fromStdString(match.captured(4).toStdString());
        listLyricsText.push_back(currentText);
        listLyricsTime.push_back(totalTime);
        lyricsMap.insert(totalTime,currentText);
        return true;
    }
    return false;
}
QList<QString>Lyrics::getListLyricsText()const{
    return listLyricsText;
}
QList<int> Lyrics::getListLyricsTime()const{
    return listLyricsTime;
}
QMap<int,QString>Lyrics::getMap()const{
    return lyricsMap;
}
