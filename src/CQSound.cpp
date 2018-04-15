#include <CQSound.h>
#include <QSound>
#include <QAudioDeviceInfo>
#include <CSDLSound.h>

CQSoundMgr *
CQSoundMgr::
getInstance()
{
  static CQSoundMgr *instance;

  if (! instance)
    instance = new CQSoundMgr;

  return instance;
}

void
CQSoundMgr::
setActive(bool active)
{
  active_ = active;
}

CQSoundMgr::
CQSoundMgr() :
 active_(true), qsound_(false), sdl_sound_(0)
{
  //qsound_ = QSound::isAvailable();
  qsound_ = ! QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty();
}

CQSound *
CQSoundMgr::
addSound(const char *filename)
{
  CQSound *sound = new CQSound(filename, qsound_);

  sounds_.push_back(sound);

  return sound;
}

void
CQSoundMgr::
playSound(CQSound *sound)
{
  if (! active_) return;

  SoundList::const_iterator p1, p2;

  if (qsound_) {
    for (p1 = sounds_.begin(), p2 = sounds_.end(); p1 != p2; ++p1)
      (*p1)->stop();
  }

  sound->play();
}

//--------------

CQSound::
CQSound(const char *filename, bool qsound) :
 filename_(filename), qsound_(0)
{
  if (qsound)
    qsound_ = new QSound(filename);
  else
    sound_ = CSDLSoundMgrInst->createSound(filename);
}

CQSound::
~CQSound()
{
  delete qsound_;
  delete sound_;
}

void
CQSound::
play()
{
  if (qsound_)
    qsound_->play();
  else
    sound_->play();
}

void
CQSound::
stop()
{
  if (qsound_)
    qsound_->stop();
  else
    sound_->stop();
}
