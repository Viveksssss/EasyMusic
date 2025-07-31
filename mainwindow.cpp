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
#include "settings.h"

#include <QPainterPath>
#include <QRegion>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mode(ORDER_MODE)
{
    ui->setupUi(this);
    init();
    connections();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initResource()
{
    this->setMouseTracking(true);
    setWindowTitle("Music");
    setWindowIcon(QIcon(""));
    setBackground(":/resource/background.png");

    output = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player-> setAudioOutput(output);

    timer = new QTimer(this);
    timer->setInterval(100);
    timer->start();

    slider = new QSlider(Qt::Horizontal,this);
    process = new QLabel(this);

    lyricsWidget = new LyricsWidget(this);

    volume = new QPushButton(this);

    sliderForVolume = new QSlider(this);

    listObj = new QListWidget(this);

    previous = new QPushButton();
    play = new QPushButton();
    next = new QPushButton();
    modes = new QPushButton();
    list = new QPushButton();


}

void MainWindow::init()
{
    // 初始化资源
    initResource();

    // 设置进度显示
    process->setText("00:00/00:00");
    QPalette palette = process->palette();
    palette.setColor(QPalette::WindowText, QColor("#FF66CC"));
    process->setPalette(palette);


    // 设置歌词显示控件
    lyricsWidget->setFixedWidth(700);
    lyricsWidget->setFixedHeight(300);


    // 设置音量标志
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


    // 设置音量Slider
    sliderForVolume->setRange(0,100);
    sliderForVolume->setValue(100);
    sliderForVolume->hide();
    sliderForVolume->setFixedHeight(80);
    sliderForVolume->setOrientation(Qt::Vertical);
    sliderForVolume->setStyleSheet("QSlider::groove:vertical{"
                          "background: #ccc; "
                          "border-radius: 5px; "
                                   "height:80px"
                                   "width:5px"
                          "margin: 0 5px;"
                          "}"
                          );

    // 设置播放列表
    listObj->setFixedWidth(300);
    listObj->setFixedHeight(this->height()-200);
    listObj->setStyleSheet(
                            "QListWidget{"
                            "background-color: rgba(255, 255, 255, 0.5);"
                           "border:none;"
                           "border-radius:20;"
                            "}"
                           // 单个项的样式
                           "QListWidget::item {"
                           "   background-color: rgba(255, 255, 255, 0.3);"
                           "   border-radius: 10px;"
                           "   padding: 8px 12px;"
                           "   margin: 4px 0;"
                           "   color: #333;"
                           "  min-height: 50px; "
                           "   height:50px;"
                           "   font-size: 19px;"
                           "}"

                           // 鼠标悬停项样式
                           "QListWidget::item:hover {"
                           "   background-color: rgba(255, 255, 255, 0.5);"
                           "   color: #000;"
                           "}"

                           // 选中项样式
                           "QListWidget::item:selected {"
                           "   background-color: rgba(255, 102, 204, 0.7);"
                           "   color: white;"
                           "   border: 1px solid rgba(255, 102, 204, 0.9);"
                           "}"

                           );
    // listObj->setAttribute(Qt::WA_TranslucentBackground);
    listObj->setFrameShape(QFrame::NoFrame);
    listObj->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    setMask(listObj,20);
    listObj->setUniformItemSizes(true);
    listObj->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listObj->hide();  // 初始隐藏
    listObj->move(this->width(), 0);
    listObj->setCurrentRow(0);

    // 初始化按钮
    initButton();

    // 确定控件位置
    position();

    // 加载列表
    loadMusic(Settings::loadLastUsedDirectory());
}

void MainWindow::connections()
{
    /* 定时器检测播放状态 */
    connect(timer,&QTimer::timeout,this,[this](){
        if(player->playbackState() == QMediaPlayer::PlayingState){
            play->setIcon(QIcon(":/resource/pause.png"));
        }else{
            play->setIcon(QIcon(":/resource/play.png"));
        }
    });

    /* 列表点击 */
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

    /* 滑动条设置 */
    sliderFunc();

    /* 按钮连接 */
    int i = 0;
    for(const auto &p:btnList){
        p->setProperty("btnType",i++);
        connect(p,&QPushButton::clicked,this,&MainWindow::connectionsForButton);
    }

}


void MainWindow::initButton()
{

    // 播放列表案件
    list->installEventFilter(this);

    // 设置动画
    animation = new QPropertyAnimation(listObj, "pos", this);
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    btnList << previous << play << next << modes << list;

    // 添加按钮布局
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(previous);
    hlayout->addWidget(play);
    hlayout->addWidget(next);
    hlayout->addWidget(modes);
    hlayout->addWidget(list);
    hlayout->setSpacing(20);
    widget = new QWidget;
    widget->setParent(this);
    widget->setLayout(hlayout);
    setButtonStyle(previous,":/resource/previous.png");
    setButtonStyle(play,":/resource/play.png");
    setButtonStyle(next,":/resource/next.png");
    setButtonStyle(modes,":/resource/order.png");
    setButtonStyle(list,":/resource/list.png");
}


/* 设置按钮风格 */
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
                border-radius :10;
            }
            QPushButton:pressed{
                background:lightpink;
                border-radius:10;
            }
    )");
}


/* 设置应用背景 */
void MainWindow::setBackground(const QString &filename){
    this->setAutoFillBackground(true);
    setStyleSheet(QString("QMainWindow{border-image:url(%1);}")
                      .arg(filename));
}



/* 确定控件位置 */
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

/* 信号连接 */
void MainWindow::connectionsForButton(){
    QPushButton*btn = static_cast<QPushButton*>(sender());
    int tmp = btn->property("btnType").toInt();
    switch(tmp){
        case 0:
            btnForPrevious();
            break;
        case 1:
            btnForPlay();
            break;
        case 2:
            btnForNext();
            break;
        case 3:
            btnForModes();
            break;
        case 4:
            btnForList();
            break;
    }
}

/* 音量标志设置 */
void MainWindow::volumeFunc()
{
    if(volume->property("status").toString() == "yes"){
        // 静音操作：记录当前音量（即使为0也记录）
        int currentVolume = sliderForVolume->value();
        volume->setProperty("volume", currentVolume); // 保存当前值

        volume->setIcon(QPixmap(":/resource/noVolume.png"));
        volume->setProperty("status","no");
        output->setVolume(0);
        sliderForVolume->setValue(0);
    }else{
        // 取消静音：取出之前保存的音量值
        int savedVolume = volume->property("volume").toInt();
        // 如果保存的音量为0，设置默认值50
        if(savedVolume <= 0) savedVolume = 50;

        volume->setIcon(QPixmap(":/resource/volume.png"));
        volume->setProperty("status","yes");
        output->setVolume(savedVolume/100.0);
        sliderForVolume->setValue(savedVolume);

        qDebug() << "恢复音量值:" << savedVolume;
    }
}

void MainWindow::setMask(QListWidget *listObj, int radius)
{
    if (!listObj || !listObj->viewport()) return;

    // 获取 viewport 尺寸
    QRect r = listObj->viewport()->rect();

    // 构建圆角矩形路径

    QPainterPath path;
    path.addRoundedRect(r, radius, radius);

    // 生成 QRegion 掩码
    QRegion mask = QRegion(path.toFillPolygon().toPolygon());

    // 应用到 viewport
    listObj->viewport()->setMask(mask);
}

/* 加载列表 */
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


/* 歌曲切换 */
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

void MainWindow::btnForPrevious()
{
    if(listObj->count() == 0)
        return;
    int curRow = listObj->currentRow();
    int nextRow = 0;
    if(mode == ORDER_MODE){
        nextRow = (curRow == 0)? listObj->count()-1 :curRow-1;
        if(nextRow<0){
            curRow = 0;
            return;
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
}

void MainWindow::btnForPlay()
{
    if(player->playbackState() == QMediaPlayer::PlayingState){
        timer->stop();
        player->pause();
        setButtonStyle(play,":/resource/play.png");
    }else{
        player->play();
        timer->start();
        setButtonStyle(play,":/resource/pause.png");
    }
}

void MainWindow::btnForNext()
{
    if(listObj->count() == 0)
        return;
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
}

void MainWindow::btnForModes()
{
    if(mode == ORDER_MODE){
        mode = RANDOM_MODE;
        modes->setIcon(QIcon(":/resource/shuffle.png"));
    }else if(mode == RANDOM_MODE){
        mode = CIRCLE_MODE;
        modes->setIcon(QIcon(":/resource/circle.png"));
    }else{
        mode = ORDER_MODE;
        modes->setIcon(QIcon(":/resource/order.png"));
    }
}

void MainWindow::btnForList()
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


/* 进度条+音量设置 */
void MainWindow::sliderFunc(){
    // 进度条连接
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

    // 设置音量键标志
    connect(volume,&QPushButton::clicked,this,[this](){
        volumeFunc();
    });

    // 音量设置
    connect(sliderForVolume,&QSlider::valueChanged,this,[this](int value){
        volume->setProperty("volume",value);
        output->setVolume(value/100.0);

        if(value == 0 && volume->property("status").toString() == "yes") {
            volumeFunc();
        }
        // 如果滑块从0被调高，自动取消静音
        else if(value > 0 && volume->property("status").toString() == "no") {
            volumeFunc();
        }
    });
}



/* 更新歌词 */
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

/* 点击其他部分关闭列表 */
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

/* 设置音量标志hover */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mousePos = event->pos();
    QRect volumeRect = volume->geometry();
    QRect sliderRect = sliderForVolume->geometry();

    bool shouldShow = volumeRect.contains(mousePos) ||
            (sliderForVolume->isVisible() && sliderRect.contains(mousePos));

    if (shouldShow && !sliderForVolume->isVisible()) {
        sliderForVolume->show();
        sliderForVolume->raise();
    } else if (!shouldShow && sliderForVolume->isVisible()) {
        sliderForVolume->hide();
    }

    QWidget::mouseMoveEvent(event);
}

/* 注册双击事件 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{

    if (obj == list  && event->type() == QEvent::MouseButtonPress) {
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
                    Settings::saveLastUsedDirectory(path);
                    loadMusic(path);
                    lastClickTime = 0;
                    return true;
                }
                lastClickTime = currentTime;
            }
        }
        return QMainWindow::eventFilter(obj, event);
}
