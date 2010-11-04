#include "outputview.h"

OutputView::OutputView(QWidget* parent): DisplayFrame(parent)
{
}

OutputView::~OutputView()
{
}

void OutputView::updateDisplay(FramePtr f)
{
    if (f != of) {
        of = f;
        setFrame(f);
    }
}
