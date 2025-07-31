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
    // 播放状态
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
    qint64 lastClickTime = 0;
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


    QPushButton*previous;
    QPushButton*play;
    QPushButton*next;
    QPushButton*modes;
    QPushButton*list;



protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event)override;
    void mousePressEvent(QMouseEvent *event)override;



private:
    void initResource();
    void initButton();
    void init();

    /* 信号 */
    void connections();
    /* 按钮风格设置 */
    void setButtonStyle(QPushButton*button,const QString &filename);
    /* 设置背景 */
    void setBackground(const QString &filename);
    /* 确定控件位置 */
    void position();
    /* 按钮的连接 */
    void connectionsForButton();
    /* 加载列表 */
    void loadMusic(const QString &path);
    /* 进度条+音量条 */
    void sliderFunc();
    /* 更新歌词 */
    void updateLyrics(qint64 position);
    /* 歌曲切换 */
    void change(int nextRow);
    /* 音量标志设置 */
    void volumeFunc();
    /* 裁剪 */
    void setMask(QListWidget*list,int radius);

    /* 上一首按钮槽函数 */
    void btnForPrevious();
    /* 播放按钮槽函数 */
    void btnForPlay();
    /* 下一首按钮槽函数 */
    void btnForNext();
    /* 模式按钮槽函数 */
    void btnForModes();
    /* 列表按钮槽函数 */
    void btnForList();

};
#endif // MAINWINDOW_H
