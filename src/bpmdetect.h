#ifndef BPMDETECT_H
#define BPMDETECT_H

#include <QAudioDecoder>

#include <soundtouch/BPMDetect.h>


class BPMDetect : public QAudioDecoder
{
    Q_OBJECT

public:
    explicit BPMDetect( QObject *parent = nullptr);
    ~BPMDetect();

signals:
    void bpmDetected( double value );

public slots:
    void on_bufferReady();
    void on_finished();

private://state
    soundtouch::BPMDetect *bpm = nullptr;
};

#endif // BPMDETECT_H
