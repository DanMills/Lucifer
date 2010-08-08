#include "displayframe.h"
#include "frame.h"

class OutputView : public DisplayFrame
{
    Q_OBJECT
public:
    OutputView (QWidget *parent);
    ~OutputView ();
public slots:
    void update (ConstFramePtr f);
private slots:
    void setup ();
    void clicked ();
private:
    QAction * setupAct;
    QMenu popup;
};

OutputView::OutputView(QWidget* parent): DisplayFrame(parent)
{
    setupAct = new QAction (tr("Setup"),this);
    connect(setupAct, SIGNAL(triggered()), this, SLOT(setup()));
    popup.addAction(setupAct);
}

OutputView::~OutputView()
{
}

void OutputView::update(ConstFramePtr f)
{
    setFrame(f);
}
