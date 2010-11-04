#ifndef OUTPUT_VIEW_INCL
#define OUTPUT_VIEW_INCL

#include "displayframe.h"
#include "frame.h"

class OutputView : public DisplayFrame
{
	Q_OBJECT
	public:
		OutputView (QWidget *parent);
		~OutputView ();
	public slots:
		void updateDisplay (FramePtr f);
		//private slots:
		//    void setup ();
		//    void clicked ();
	private:
		FramePtr of;
};

#endif