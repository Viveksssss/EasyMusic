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
#include <QSlider>
#include <QLabel>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mode(ORDER_MODE)
{
    ui->setupUi(this);
    this->setMouseTracking(true);
    setWindowTitle("Music");
    setWindowIcon(QIcon(""));
    setBackground(":/resource/background.png");
    initButton();
    position();
    output = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player-> setAudioOutput(output);

    QSettings settings("Vivek","myMusic");
    QString paths = settings.value("lastUse",QDir::homePath()).toString();
    qDebug() << paths;
    if(paths.isNull()){
        paths = "";
    }
    loadMusic(paths);

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
        lyrics.lyricsPath = path+"/"+item->text()+".lrc";
        lyrics.readLyricsFile(lyrics.lyricsPath);
        lyricsMap.clear();
        lyricsMap = lyrics.getMap();
        totalPosition = player->duration();
        all = QTime(0,0);
        all  = all.addMSecs(totalPosition);
        process->setText("00:00/" + all.toString("mm:ss"));
        change(-1);
    });

    sliderFunc();

    connect(sliderForVolume,&QSlider::valueChanged,this,[this](int value){
        volume->setProperty("volume",value);
        output->setVolume(value/100.0);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initButton()
{
    slider = new QSlider(Qt::Horizontal,this);
    process = new QLabel(this);
    process->setText("00:00/00:00");
    QPalette palette = process->palette();
    palette.setColor(QPalette::WindowText, QColor("#FF66CC"));
    process->setPalette(palette);

    lyricsWidget = new LyricsWidget(this);
    lyricsWidget->setFixedWidth(700);
    lyricsWidget->setFixedHeight(300);

    volume = new QPushButton(this);
    volume->setIcon(QPixmap(":/resource/volume.png"));
    volume->setFixedHeight(30);
    volume->setFixedWidth(30);
    volume->setProperty("status","yes");
    volume->setProperty("volume",100);
    volume->setStyleSheet("QPushButton:hover{"
                          "background-color:lightpink"
                          "}"
                          "QPushButton:press{"
                          "background-color:white"
                          "}"
                          "QPushButton{"
                          "background-color: transparent;"
                          "}");

    sliderForVolume = new QSlider(this);
    sliderForVolume->setRange(0,100);
    sliderForVolume->setValue(100);
    sliderForVolume->hide();
    sliderForVolume->setFixedHeight(80);  // 总高度为40像素
    sliderForVolume->setOrientation(Qt::Vertical);
    sliderForVolume->setStyleSheet("QSlider::groove:vertical{"
                          "background: #ccc; "
                          "border-radius: 5px; "
                                   "height:80px"
                                   "width:5px"
                          "margin: 0 5px;"
                          "}"
                          );


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
            QPushButton {
                background-color:transparent;
                border:none;
            }
            QPushButton:hover{
                background:white;
            }
            QPushButton:pressed{
                background:lightpink;
            }
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
                    QSettings settings("Vivek","myMusic");
                    settings.setValue("lastUse",path);
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
    // list
    float w = this->width();
    float h = this->height();
    float gap = 50;
    float ws = widget->sizeHint().width();
    float hs = widget->sizeHint().height();
    float x = (w - ws)/2;
    float y = (h - gap - hs);
    widget->setGeometry(QRect(x,y,ws,hs));

    // slider
    slider->setSingleStep(1);
    slider->setFixedWidth(300);
    ws = slider->width();
    hs = slider->height();
    gap = 10;
    x = (w - ws)/2;
    y = (h - gap - hs);
    slider->setGeometry(QRect(x,y,ws,hs));

    ws = volume->width();
    hs = volume->height();
    volume->setGeometry(QRect(x-ws-20,y-1,ws,hs));

    ws = sliderForVolume->width();
    hs = sliderForVolume->height();
    sliderForVolume->setGeometry(QRect(x-ws+15,y-80,ws,hs));

    ws =process ->width();
    hs = process->height();
    process->setGeometry(QRect(x+10+slider->width(),y-1,ws,hs));

    ws = lyricsWidget->width();
    hs = lyricsWidget->height();
    x = (w - ws)/2;
    y = (0+80);
    lyricsWidget->setGeometry(QRect(x,y,ws,hs));

}


void MainWindow::connections(){
    QPushButton*btn = static_cast<QPushButton*>(sender());
    int tmp = btn->property("btnType").toInt();
    switch(tmp){
            case 0:{
            if(listObj->count() == 0)
                break;
            int curRow = listObj->currentRow();
            int nextRow = 0;
            if(mode == ORDER_MODE){
                nextRow = (curRow == 0)? listObj->count()-1 :curRow-1;
                if(nextRow<0){
                    curRow = 0;
                    break;
                }
            }
            else if(mode == RANDOM_MODE){
                do{
                    nextRow =QRandomGenerator::global()->bounded(0,listObj->count());
                }while(nextRow == curRow);
            }
            else{
                nextRow = curRow;
            }
            change(nextRow);
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
            if(listObj->count() == 0)
                break;
            int curRow = listObj->currentRow();
            int nextRow = 0;
            if(mode == ORDER_MODE){
                nextRow = curRow == listObj->count()-1? 0 :curRow+1;
            }
            else if(mode == RANDOM_MODE){
                do{
                    nextRow =QRandomGenerator::global()->bounded(0,listObj->count());
                }while(nextRow == curRow&&listObj->count()!=1);
            }
            else{
                nextRow = curRow;
            }
            change(nextRow);
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

void MainWindow::change(int nextRow){
    if(nextRow != -1)
        listObj->setCurrentRow(nextRow);
    player->setSource(QUrl(path+"/"+listObj->currentItem()->text()+".mp3"));
    player->play();
    totalPosition = player->duration();
    all = QTime(0,0);
    all  = all.addMSecs(totalPosition);
    process->setText("00:00/"+all.toString("mm:ss"));

    lyrics.lyricsPath = path+"/"+listObj->currentItem()->text()+".lrc";
    lyrics.readLyricsFile(lyrics.lyricsPath);
    lyricsMap.clear();
    lyricsMap = lyrics.getMap();

    qDebug() << "Loaded lyrics count:" << lyricsMap.size();
}

void MainWindow::volumeFunc()
{
    if(volume->property("status").toString() == "yes"){
        volume->setIcon(QPixmap(":/resource/noVolume.png"));
        volume->setProperty("status","no");
        output->setVolume(0);
        sliderForVolume->setValue(0);
    }else{
        volume->setIcon(QPixmap(":/resource/volume.png"));
        volume->setProperty("status","yes");
        output->setVolume(volume->property("volume").toInt()/100.0);
        sliderForVolume->setValue(volume->property("volume").toInt());
        // sliderForVolume->setValue(100);
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


void MainWindow::sliderFunc(){


    connect(player,&QMediaPlayer::durationChanged,this,[=](qint64 duration){
        slider->setRange(0,static_cast<int>(duration));
        QTime total(0,0);
        total = total.addMSecs(duration);
        all = QTime(0, 0).addMSecs(duration);
        process->setText(total.toString("mm:ss")+"/"+all.toString("mm:ss"));
    });
    connect(player,&QMediaPlayer::positionChanged,this,[=](qint64 position){
        slider->setValue(static_cast<int>(position));
        QTime currentTime(0, 0);
        currentTime = currentTime.addMSecs(position);
        process->setText(currentTime.toString("mm:ss")+"/"+all.toString("mm:ss"));
    });
    connect(slider,&QSlider::sliderMoved,this,[=](int position){
        player->setPosition(static_cast<qint64>(position));
    });
    // 在播放器初始化时连接信号
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::updateLyrics);

    connect(volume,&QPushButton::clicked,this,[this](){
        volumeFunc();
    });
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (volume->geometry().contains(event->position().toPoint())) {
        sliderForVolume->raise();
        sliderForVolume->setValue(volume->property("volume").toInt());
        sliderForVolume->show();
    } else if(!volume->geometry().contains(event->position().toPoint())){
        // 鼠标不在按钮上
        sliderForVolume->hide();
    }
    QWidget::mouseMoveEvent(event);
}


void MainWindow::updateLyrics(qint64 position){

    if (lyricsMap.isEmpty()) return;
    auto it = lyricsMap.lowerBound(position)-1;
    QString prevLine, currentLine, nextLine;
    if (it != lyricsMap.begin()) {
           prevLine = (it - 1).value();  // 上一行歌词
       }
    else prevLine = "";

   if (it != lyricsMap.end()) {
       currentLine = it.value();      // 当前行歌词
       ++it;
       if (it != lyricsMap.end()) {
           nextLine = it.value();     // 下一行歌词
       }
   }
   else {
       currentLine = "";
       nextLine = "";
   }

    lyricsWidget->setLyrics(prevLine, currentLine, nextLine);

}


void MainWindow::mousePressEvent(QMouseEvent *event){
    if (!listObj->geometry().contains(volume->mapFromParent(event->pos()))) {
        // 隐藏列表：滑出到右侧
        QPropertyAnimation*animations = new QPropertyAnimation(listObj,"pos",this);
        animations->setStartValue(QPoint(width() - listObj->width(), 0));
        animations->setEndValue(QPoint(width(), 0));
        animations->setDuration(300);
        // 动画完成后隐藏
        connect(animations, &QPropertyAnimation::finished, this, [this](){
            listObj->hide();
        });
        animations->start();
    }
    QWidget::mousePressEvent(event);
}
