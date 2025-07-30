#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class QMediaPlayer;
class QAudioOutput;
class QListWidget;
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

};
#endif // MAINWINDOW_H
