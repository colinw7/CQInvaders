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
  CSpaceInvaders* invaders_ { nullptr };
  int             w_        { -1 };
  int             h_        { -1 };
};
