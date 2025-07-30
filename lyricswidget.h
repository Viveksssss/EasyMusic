#ifndef LYRICSWIDGET_H
#define LYRICSWIDGET_H

#include <QWidget>

class LyricsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LyricsWidget(QWidget *parent = nullptr);


    void setLyrics(const QString&pre,const QString&cur,const QString&next);
private:
    QString previous;
    QString current;
    QString next;
signals:


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // LYRICSWIDGET_H
