#include <QWidget>

class CSpaceInvaders;

class CQSpaceInvaders : public QWidget {
  Q_OBJECT

 public:
  CQSpaceInvaders();

  void resizeEvent(QResizeEvent *);

  void paintEvent(QPaintEvent *);

  void keyPressEvent(QKeyEvent *e);

 public slots:
  void timerSlot();

 private:
  int             w_, h_;
  CSpaceInvaders *invaders_;
};
