#include "bpmdetect.h"

#include <QApplication>
#include <QAudioDecoder>
#include <QFileInfo>

BPMDetect::BPMDetect( QObject *parent ) :
    QAudioDecoder( parent )
{
    connect( this, SIGNAL( bufferReady() ), this, SLOT( on_bufferReady() ) );
    connect( this, SIGNAL( finished()    ), this, SLOT( on_finished()    ) );
}

BPMDetect::~BPMDetect()
{
    if( bpm )delete bpm;
}

void
BPMDetect::on_bufferReady()
{
    while( bufferAvailable() )
    {
        auto buf = read();
        if( buf.format().sampleType() != QAudioFormat::Float ){
            qDebug() << "invalid sampleType";
            return;
        }
        if( bpm == nullptr ){
            qDebug() << "creating new BPMDetect using:";
            qDebug() << buf.format();
            bpm = new soundtouch::BPMDetect(
                        buf.format().channelCount(),
                        buf.format().sampleRate() );
        }
        bpm->inputSamples(
                    static_cast< float *>( buf.data() ),
                    buf.sampleCount() / buf.format().channelCount()
                    );
    }
}

void
BPMDetect::on_finished()
{
    qDebug() << "on_finished";
    auto bpmValue = bpm->getBpm();
    delete bpm; bpm = nullptr;

    if (bpmValue > 0){
        qDebug() << "Detected BPM rate: " << bpmValue;
        emit bpmDetected( bpmValue );
    }
    else qDebug("Couldn't detect BPM rate.");

    return;
}
