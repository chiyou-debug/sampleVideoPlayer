#include "mainwidget.h"
#include "ui_mainwidget.h"

// VLC事件回调
void PositionChanged(const struct libvlc_event_t *p_event, void *p_data)
{
    mainWidget* pMain = static_cast<mainWidget*>(p_data);
    if (p_event->type == libvlc_MediaPlayerPositionChanged) {
        pMain->ui->progressSlider->show();
        float pos = libvlc_media_player_get_position(pMain->m_vlcPlayer);
        pMain->ui->progressSlider->setValue(pos * 1000);
    }
}

mainWidget::mainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainWidget)
{
    ui->setupUi(this);

    // 初始化窗口
    this->setWindowTitle("My VLC Player");
    this->setMinimumSize(QSize(1310, 760));
    ui->voiceSlider->setValue(50);
    ui->progressSlider->setRange(0, 1000);
    ui->progressSlider->hide();

    // 创建VLC实例
    m_vlcInstance = libvlc_new(0, NULL);
    assert(m_vlcInstance != NULL);
    m_vlcPlayer = libvlc_media_player_new(m_vlcInstance);
    assert(m_vlcPlayer != NULL);

    // 音量调节
    ui->voiceSlider->installEventFilter(this);
    connect(ui->voiceSlider, &QSlider::valueChanged, this, &mainWidget::changeVolume);
    connect(ui->voiceSlider, &QSlider::sliderMoved, this, &mainWidget::changeVolume);

    // 进度条调节
    ui->progressSlider->installEventFilter(this);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &mainWidget::updateTimeLable);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &mainWidget::changePosition);
    // connect(ui->progressSlider, &QSlider::valueChanged, this, &mainWidget::changePosition); 使用valueChanged会卡顿, 但是sliderMoved就不会, 不知道咋回事, 没有发现异常

    // 监测VLC事件, 移动Slider
    m_vlcEventManager = libvlc_media_player_event_manager(m_vlcPlayer);
    libvlc_event_attach(m_vlcEventManager, libvlc_MediaPlayerPositionChanged, PositionChanged, this);
}

void mainWidget::on_importVideoBtn_clicked()
{
    // 导入视频
    QString fileName = QFileDialog::getOpenFileName(this, tr("请选择文件"), QString(QStandardPaths::DownloadLocation), tr("视频文件(*.mp4 *.mp3 *.flv);;所有文件(*.*)"));
    if (fileName == NULL) return;
    fileName = QDir::toNativeSeparators(fileName);

    // 设置路径
    m_vlcMedia = libvlc_media_new_path(m_vlcInstance, fileName.toStdString().c_str());
    assert(m_vlcMedia != NULL);

    // 解析media
    libvlc_media_parse_with_options(m_vlcMedia, libvlc_media_parse_local, -1);

    // 设置player
    libvlc_media_player_set_media(m_vlcPlayer, m_vlcMedia);

    // 设置播放的窗口句柄
    libvlc_media_player_set_hwnd(m_vlcPlayer, (void*)(ui->videoWiget->winId()));

    // 播放视频
    libvlc_media_player_play(m_vlcPlayer);
    ui->playOrPauseVideoBtn->setText(tr("暂停"));
    ui->timeLabel->setText("00:00:00 / 00:00:00");
}

void mainWidget::on_playOrPauseVideoBtn_clicked()
{
    if (m_vlcMedia == NULL) {
        QMessageBox::warning(this, "My VLC Player", "请先导入视频");
        return;
    }

    switch (libvlc_media_player_get_state(m_vlcPlayer)){
        case libvlc_state_t::libvlc_Playing:
            libvlc_media_player_pause(m_vlcPlayer);
            ui->playOrPauseVideoBtn->setText(tr("播放"));
            break;
        case libvlc_state_t::libvlc_Paused:
            libvlc_media_player_play(m_vlcPlayer);
            ui->playOrPauseVideoBtn->setText(tr("暂停"));
            break;
        default:
            break;
    }
}

void mainWidget::on_muteVideoBtn_clicked()
{
    if (m_vlcMedia == NULL) {
        QMessageBox::warning(this, "My VLC Player", "请先导入视频");
        return;
    }

    if (ui->muteVideoBtn->text() == "静音") {
        libvlc_audio_set_mute(m_vlcPlayer, true);
        ui->muteVideoBtn->setText("解除静音");
        ui->voiceSlider->setValue(0);
    } else {
        libvlc_audio_set_mute(m_vlcPlayer, false);
        ui->muteVideoBtn->setText("静音");
        ui->voiceSlider->setValue(50);
    }
}

void mainWidget::changeVolume(int vol)
{
    libvlc_audio_set_volume(m_vlcPlayer, vol);
}

void mainWidget::changePosition(int pos)
{
    libvlc_media_player_set_position(m_vlcPlayer, (float)pos / 1000.0);
}

void mainWidget::updateTimeLable()
{
    char buf[256];

    // 获取视频总时长并格式化
    libvlc_time_t totalSecs = libvlc_media_get_duration(m_vlcMedia) / 1000;
    int totalHour = totalSecs / 3600;
    int totalMinute = (totalSecs - totalHour * 3600 ) / 60;
    int totalSec = totalSecs - totalHour * 3600 - totalMinute * 60;
    sprintf(buf, "%02d:%02d:%02d", totalHour, totalMinute, totalSec);
    QString totalTimeFormat(buf);

    // 获取当前时长并格式化
    libvlc_time_t curSecs = libvlc_media_player_get_time(m_vlcPlayer) / 1000;
    int curHour = curSecs / 3600;
    int curMinute = (curSecs - curHour * 3600 ) / 60;
    int curSec = curSecs - curHour * 3600 - curMinute * 60;
    sprintf(buf, "%02d:%02d:%02d", curHour, curMinute, curSec);
    QString curTimeFormat(buf);

    ui->timeLabel->setText(curTimeFormat + " / " + totalTimeFormat);
}

mainWidget::~mainWidget()
{
    // 释放资源
    libvlc_media_release(m_vlcMedia);
    libvlc_media_player_release(m_vlcPlayer);
    libvlc_release(m_vlcInstance);
    m_vlcMedia = NULL;
    m_vlcPlayer = NULL;
    m_vlcInstance = NULL;

    delete ui;
}

bool mainWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->progressSlider || obj == ui->voiceSlider)
    {
        QSlider *slider =(QSlider*)obj;
        if (event->type()==QEvent::MouseButtonPress)           //判断类型
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)       //判断左键
            {
               int dur = slider->maximum() - ui->progressSlider->minimum();
               int pos = slider->minimum() + dur * ((double)mouseEvent->x() / slider->width());
               if(pos != slider->sliderPosition())
                   slider->setValue(pos);
            }
        }
    }
    return QObject::eventFilter(obj,event);
}
