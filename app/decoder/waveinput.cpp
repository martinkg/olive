#include "waveinput.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <QDataStream>

WaveInput::WaveInput(const QString &f) :
  file_(f)
{
}

WaveInput::~WaveInput()
{
  close();
}

bool WaveInput::open()
{
  if (!file_.open(QFile::ReadOnly)) {
    return false;
  }

  if (file_.read(4) != "RIFF") {
    close();
    qDebug() << "No RIFF found";
    return false;
  }

  // Skip filesize bytes
  file_.seek(file_.pos() + 4);

  if (file_.read(4) != "WAVE") {
    close();
    qDebug() << "No WAVE found";
    return false;
  }

  // Find fmt_ section
  if (!find_str(&file_, "fmt ")) {
    close();
    qDebug() << "No fmt  found";
    return false;
  }

  // Skip fmt_ section size
  file_.seek(file_.pos()+4);

  // Create data stream for reading bytes into types
  QDataStream data_stream(&file_);
  data_stream.setByteOrder(QDataStream::LittleEndian);

  // Read data type
  uint16_t data_type;
  data_stream >> data_type;

  bool data_is_float;
  switch (data_type) {
  case 1: // PCM Integer
    data_is_float = false;
    break;
  case 3:
    data_is_float = true;
    break;
  default:
    // If it's neither float nor int, we can't work with this file
    close();
    qDebug() << "Invalid WAV type" << data_type;
    return false;
  }

  // Read number of channels
  uint16_t channel_count;
  data_stream >> channel_count;

  uint64_t channel_layout = static_cast<uint64_t>(av_get_default_channel_layout(channel_count));

  int32_t sample_rate;
  data_stream >> sample_rate;

  // Skip bytes per second value and bytes per sample value
  file_.seek(file_.pos() + 6);

  uint16_t bits_per_sample;
  data_stream >> bits_per_sample;

  SampleFormat format;

  switch (bits_per_sample) {
  case 8:
    format = SAMPLE_FMT_U8;
    break;
  case 16:
    format = SAMPLE_FMT_S16;
    break;
  case 32:
    if (data_is_float) {
      format = SAMPLE_FMT_S32;
    } else {
      format = SAMPLE_FMT_FLT;
    }
    break;
  case 64:
    if (data_is_float) {
      format = SAMPLE_FMT_S64;
    } else {
      format = SAMPLE_FMT_DBL;
    }
    break;
  default:
    // We don't know this format...
    close();
    qDebug() << "Invalid format found" << bits_per_sample;
    return false;
  }

  // We're good to go!
  params_ = AudioRenderingParams(sample_rate, channel_layout, format);

  if (!find_str(&file_, "data")) {
    close();
    qDebug() << "No data tag found";
    return false;
  }

  data_position_ = file_.pos() + 4;

  return true;
}

bool WaveInput::is_open()
{
  return file_.isOpen();
}

QByteArray WaveInput::read(int offset, int length)
{
  if (!is_open()) {
    return QByteArray();
  }

  file_.seek(offset + data_position_);
  return file_.read(length);
}

void WaveInput::read(int offset, char *buffer, int length)
{
  if (!is_open()) {
    return;
  }

  file_.seek(offset + data_position_);
  file_.read(buffer, length);
}

AudioRenderingParams WaveInput::params()
{
  return params_;
}

void WaveInput::close()
{
  if (file_.isOpen()) {
    file_.close();
  }
}

bool WaveInput::find_str(QFile *f, const char *str)
{
  qint64 pos = f->pos();
  while (f->read(4) != str) {
    if (f->atEnd()) {
      return false;
    }

    pos++;
    f->seek(pos);
  }

  return true;
}
