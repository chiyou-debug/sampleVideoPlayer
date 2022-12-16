#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDebug>
#include <QSlider>
#include <vlc/vlc.h>

QT_BEGIN_NAMESPACE
namespace Ui { class mainWidget; }
QT_END_NAMESPACE

class mainWidget : public QWidget
{
    Q_OBJECT

    friend void PositionChanged(const struct libvlc_event_t *p_event, void *p_data );

public:
    mainWidget(QWidget *parent = nullptr);
    ~mainWidget();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void on_importVideoBtn_clicked();
    void on_playOrPauseVideoBtn_clicked();
    void on_muteVideoBtn_clicked();

    void changeVolume(int vol);
    void changePosition(int pos);
    void updateTimeLable();

private:
    Ui::mainWidget *ui;

    libvlc_instance_t *m_vlcInstance;
    libvlc_media_player_t *m_vlcPlayer;
    libvlc_media_t *m_vlcMedia;
    libvlc_event_manager_t * m_vlcEventManager;
};
#endif // MAINWIDGET_H
