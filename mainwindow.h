#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include "lyrices.h"
#include "lyricswidget.h"
#include <QToolButton>

class QPushButton;
class QMediaPlayer;
class QAudioOutput;
class QListWidget;
class QSlider;
class QLabel;
class QPropertyAnimation;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    enum PLAYERMODE{
        ORDER_MODE,
        RANDOM_MODE,
        CIRCLE_MODE
    };

    PLAYERMODE mode;
    Ui::MainWindow *ui;
    QWidget*widget;
    QMediaPlayer *player;
    QAudioOutput *output;
    QList<QPushButton*>btnList;
    QListWidget *listObj;
    QString path;
    QPropertyAnimation*animation;
    QTimer*timer;
    QPushButton*play;
    qint64 lastClickTime = 0;
    QPushButton*list;
    QSlider*slider;
    qint64 totalPosition;
    QTime all;
    Lyrics lyrics;
    QString lyricsName;
    LyricsWidget*lyricsWidget;
    QMap<int,QString>lyricsMap;
    QPushButton *volume;
    QLabel*process;
    QSlider *sliderForVolume;
protected:
void resizeEvent(QResizeEvent *event)override;
void showEvent(QShowEvent *event)override;
bool eventFilter(QObject *obj, QEvent *event) override;



private:
    void initButton();
    void setButtonStyle(QPushButton*button,const QString &filename);
    void setBackground(const QString &filename);
    void position();
    void connections();
    void loadMusic(const QString &path);
    void onList(QPushButton*btn);
    void sliderFunc();
    void updateLyrics(qint64 position);
    void change(int nextRow);
    void volumeFunc();
    void mouseMoveEvent(QMouseEvent *event)override;



    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event);
};
#endif // MAINWINDOW_H
