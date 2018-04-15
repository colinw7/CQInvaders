#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <CQSpaceInvaders.h>
#include <CQSound.h>

struct Image {
  QImage image;

  Image(QImage image1) :
   image(image1) {
  }
};

struct Sound {
  CQSound *sound;

  Sound(CQSound *sound1) :
   sound(sound1) {
  }
};

class App {
 public:
  static Image *loadImage(const char *filename);

  static Sound *loadSound(const char *filename);

  static void drawImage(int x, int y, Image *image);

  static void drawLeftText    (int x, int y, const char *str);
  static void drawCenteredText(int x, int y, const char *str);
  static void drawRightText   (int x, int y, const char *str);

  static void playSound(Sound *sound);

  static QPainter *painter;

 private:
  typedef std::map<std::string,Image *> ImageList;
  typedef std::map<std::string,Sound *> SoundList;

  static ImageList images_;
  static SoundList sounds_;
};

App::ImageList App::images_;
App::SoundList App::sounds_;

#include <CSpaceInvaders.h>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQSpaceInvaders *invaders = new CQSpaceInvaders;

  invaders->resize(800, 1000);

  invaders->show();

  return app.exec();
}

CQSpaceInvaders::
CQSpaceInvaders()
{
  setFont(QFont("Helvetica", 20));

  invaders_ = new CSpaceInvaders;

  QTimer *timer = new QTimer;

  connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));

  timer->start(1000.0/60.0);
}

void
CQSpaceInvaders::
resizeEvent(QResizeEvent *)
{
  w_ = width ();
  h_ = height();
}

void
CQSpaceInvaders::
paintEvent(QPaintEvent *)
{
  QPainter p(this);

  App::painter = &p;

  p.fillRect(rect(), QBrush(QColor(0,0,0)));

  invaders_->draw();

  App::painter = NULL;
}

void
CQSpaceInvaders::
keyPressEvent(QKeyEvent *e)
{
  if      (e->key() == Qt::Key_Left)
    invaders_->moveShipLeft();
  else if (e->key() == Qt::Key_Right)
    invaders_->moveShipRight();
  else if (e->key() == Qt::Key_Space)
    invaders_->shipFire();
  else if (e->key() == Qt::Key_P)
    invaders_->pause();
  else if (e->key() == Qt::Key_R)
    invaders_->restart();
}

void
CQSpaceInvaders::
timerSlot()
{
  invaders_->update();

  update();
}

//------

QPainter *App::painter = NULL;

Image *
App::
loadImage(const char *filename)
{
  ImageList::iterator p = images_.find(filename);

  if (p != images_.end())
    return (*p).second;

  QImage qimage;

  qimage.load(filename);

  Image *image = new Image(qimage);

  images_[filename] = image;

  return image;
}

Sound *
App::
loadSound(const char *filename)
{
  SoundList::iterator p = sounds_.find(filename);

  if (p != sounds_.end())
    return (*p).second;

  CQSound *qsound = CQSoundMgrInst->addSound(filename);

  Sound *sound = new Sound(qsound);

  sounds_[filename] = sound;

  return sound;
}

void
App::
drawImage(int x, int y, Image *image)
{
  painter->drawImage(x, y, image->image);
}

void
App::
drawLeftText(int x, int y, const char *str)
{
  QFontMetrics fm(painter->font());

  painter->setPen(QColor(255,255,255));

  painter->drawText(x, y + fm.ascent(), str);
}

void
App::
drawCenteredText(int x, int y, const char *str)
{
  QFontMetrics fm(painter->font());

  int w = fm.width(str);

  painter->setPen(QColor(255,255,255));

  painter->drawText(x - w/2, y + fm.ascent(), str);
}

void
App::
drawRightText(int x, int y, const char *str)
{
  QFontMetrics fm(painter->font());

  int w = fm.width(str);

  painter->setPen(QColor(255,255,255));

  painter->drawText(x - w, y + fm.ascent(), str);
}

void
App::
playSound(Sound *sound)
{
  CQSoundMgrInst->playSound(sound->sound);
}
