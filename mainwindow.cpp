#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QPalette>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QListWidget>
#include <QRandomGenerator64>
#include <QTimer>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mode(ORDER_MODE)
{
    ui->setupUi(this);
    ui->centralwidget->setFixedSize(800,600);


    setWindowTitle("Music");
    setWindowIcon(QIcon(""));
    setBackground(":/resource/background.png");
    initButton();
    position();
    output = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player-> setAudioOutput(output);
    player->setSource(QUrl::fromLocalFile("/home/vivek/Tmp/PVZ/Grazy Dave.mp3"));
    player->stop();

    loadMusic("/timeshift/snapshots/2025-07-29_10-00-00/localhost/opt/visual-studio-code/resources/app/out/vs/platform/accessibilitySignal/browser/media");

    timer = new QTimer(this);
    timer->setInterval(100);
    timer->start();
    connect(timer,&QTimer::timeout,this,[this](){
        if(player->playbackState() == QMediaPlayer::PlayingState){
            play->setIcon(QIcon(":/resource/pause.png"));
        }else{
            play->setIcon(QIcon(":/resource/play.png"));
        }
    });

    connect(listObj,&QListWidget::itemClicked,this,[this](QListWidgetItem*item){
        player->setSource(QUrl(path+"/"+item->text()+".mp3"));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initButton()
{
    listObj = new QListWidget(this);
    listObj->setFixedWidth(300);  // 设置列表宽度
    listObj->setFixedHeight(this->height()-200);
    listObj->setStyleSheet("background-color: rgba(255, 255, 255, 0.5);"
                           "border:none;"
                           "border-radius:20;"
                           );
    listObj->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listObj->hide();  // 初始隐藏
    listObj->move(this->width(), 0);
    listObj->setCurrentRow(0);


    QPushButton*previous = new QPushButton();
    play = new QPushButton();
    QPushButton*next = new QPushButton();
    QPushButton*mode = new QPushButton();
    list = new QPushButton();
    list->installEventFilter(this);


    animation = new QPropertyAnimation(listObj, "pos", this);
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    btnList << previous << play << next << mode << list;
    int i = 0;
    for(const auto &p:btnList){
        p->setProperty("btnType",i++);
        connect(p,&QPushButton::clicked,this,&MainWindow::connections);
    }

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(previous);
    hlayout->addWidget(play);
    hlayout->addWidget(next);
    hlayout->addWidget(mode);
    hlayout->addWidget(list);
    hlayout->setSpacing(20);
    widget = new QWidget;
    widget->setParent(this);
    widget->setLayout(hlayout);
    setButtonStyle(previous,":/resource/previous.png");
    setButtonStyle(play,":/resource/play.png");
    setButtonStyle(next,":/resource/next.png");
    setButtonStyle(mode,":/resource/order.png");
    setButtonStyle(list,":/resource/list.png");
}


void MainWindow::setButtonStyle(QPushButton*button,const QString &filename){
    button->setFixedSize(50,50);
    button->setIcon(QIcon(filename));
    button->setIconSize(QSize(button->width(),button->height()));
    button->setText("");
    button->setStyleSheet(R"(
            background-color:transparent;
    )");
}

void MainWindow::setBackground(const QString &filename){

    // ui->centralwidget->setStyleSheet("background:transparent;");
    // QPixmap pixmap(filename);
    // if(pixmap.isNull()){
    //     qDebug() << "❌ 文件加载失败：" << filename;
    //     return;
    // }
    // QSize winSize = this->size();

    // QPixmap p = pixmap.scaled(winSize,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    // QPalette pal = this->palette();
    // pal.setBrush(QPalette::Window,QBrush(p));
    this->setAutoFillBackground(true);
    // this->setPalette(pal);
    setStyleSheet(QString("QMainWindow{border-image:url(%1);}")
                      .arg(filename));
}


void MainWindow::resizeEvent(QResizeEvent *event){
    QMainWindow::resizeEvent(event);
    position();
}
void MainWindow::showEvent(QShowEvent *event){
    QMainWindow::showEvent(event);
    position();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{

    if (obj ==list  && event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                if (currentTime - lastClickTime < 500) { // 300ms内视为双击
                    qDebug() << "Double clicked!";
                    QString str = QFileDialog::getExistingDirectory(this);
                    if (str.isEmpty()) {
                        qDebug() << "Selected path:" << path;
                        return true;
                    }
                    path = str;
                    loadMusic(path);
                    lastClickTime = 0;
                    return true;
                }
                lastClickTime = currentTime;
            }
        }
        return QMainWindow::eventFilter(obj, event);
}

void MainWindow::position(){
    float w = this->width();
    float h = this->height();
    float gap = 50;
    float ws = widget->sizeHint().width();
    float hs = widget->sizeHint().height();
    float x = (w - ws)/2;
    float y = (h - gap - hs);

    widget->setGeometry(QRect(x,y,ws,hs));
    // qDebug() << "Position set to:" << x << y << ws << hs;
}


void MainWindow::connections(){
    QPushButton*btn = static_cast<QPushButton*>(sender());
    int tmp = btn->property("btnType").toInt();
    switch(tmp){
            case 0:{
            int curRow = listObj->currentRow();
            int nextRow = 0;
            if(mode == ORDER_MODE)
                nextRow = curRow == 0? listObj->count()-1 :curRow-1;
            else if(mode == RANDOM_MODE){
                do{
                    nextRow =QRandomGenerator::global()->bounded(0,listObj->count());
                }while(nextRow == curRow);
            }
            else{
                nextRow = curRow;
            }
            listObj->setCurrentRow(nextRow);
            player->setSource(QUrl(path+listObj->currentItem()->text()+".mp3"));
            break;
        }
        case 1:
            if(player->playbackState() == QMediaPlayer::PlayingState){
                timer->stop();
                player->pause();
                setButtonStyle(btn,":/resource/play.png");
            }else{
                player->play();
                timer->start();
                setButtonStyle(btn,":/resource/pause.png");
            }
            // player->play();
            break;
        case 2:{
            int curRow = listObj->currentRow();
            int nextRow = 0;
            if(mode == ORDER_MODE)
                nextRow = curRow == listObj->count()-1? 0 :curRow+1;
            else if(mode == RANDOM_MODE){
                do{
                    nextRow =QRandomGenerator::global()->bounded(0,listObj->count());
                }while(nextRow == curRow);
            }
            else{
                nextRow = curRow;
            }
            listObj->setCurrentRow(nextRow);
            player->setSource(QUrl(path+"/"+listObj->currentItem()->text()+".mp3"));
            player->play();
            break;
        }

        case 3:
            if(mode == ORDER_MODE){
                mode = RANDOM_MODE;
                btn->setIcon(QIcon(":/resource/shuffle.png"));
            }else if(mode == RANDOM_MODE){
                mode = CIRCLE_MODE;
                btn->setIcon(QIcon(":/resource/circle.png"));
            }else{
                mode = ORDER_MODE;
                btn->setIcon(QIcon(":/resource/order.png"));
            }
            break;
        case 4:
            onList(btn);
            break;
    }
}

void MainWindow::loadMusic(const QString &path)
{
    this->path = path;
    QDir dir(path);
    if(dir.exists() == false){
        QMessageBox::warning(this,"文件夹","打开失败");
        return;
    }

    QFileInfoList list = dir.entryInfoList(QDir::Files);
    if(list.empty())
        return;

    listObj->clear();
    for(auto&element:list){
        if(element.suffix() == "mp3"){
            listObj->addItem(element.baseName());
        }
    }
}

void MainWindow::onList(QPushButton*btn)
{
    // 确保listObj和animation已正确初始化
        if(!listObj || !animation) {
            qDebug() << "Error: listObj or animation not initialized!";
            return;
        }

        bool willShow = listObj->isHidden();

        // 先断开之前可能的动画完成信号连接
        disconnect(animation, &QPropertyAnimation::finished, nullptr, nullptr);

        if(willShow) {
            // 显示列表：从右侧滑入
            listObj->show();
            listObj->raise();  // 确保在最上层

            animation->setStartValue(QPoint(width(), 0));
            animation->setEndValue(QPoint(width() - listObj->width(), 0));
        } else {
            // 隐藏列表：滑出到右侧
            animation->setStartValue(QPoint(width() - listObj->width(), 0));
            animation->setEndValue(QPoint(width(), 0));

            // 动画完成后隐藏
            connect(animation, &QPropertyAnimation::finished, this, [this](){
                listObj->hide();
            });
        }

        animation->start();
}
