#ifndef VIWER_H
#define VIWER_H

#include <QtGui>
#include <QWidget>
#include <QDoubleSpinBox>
#include "QDSimulator.h"

class Viewer : public QWidget
{
Q_OBJECT                 //  Qt magic macro
private:
   string allProcessors;
   QSpinBox* state;    //  Spinbox for s
   QPushButton* play;
   QLabel* probabilityLabel;
   QSpinBox* sliceI;
   QSpinBox* sliceJ;
   QSpinBox* sliceK;
   QDSimulator* qDSimulator;
   QLabel* iteration;
   QLineEdit * processorText;
   QLabel* processorLabel;
   QSlider* minProbability;
   QCheckBox* residual;
   void updateProcessorInfo();
   QLineEdit * saveFile;   
   bool continueSameFile;
   QPrinter printer;
   QPainter painter;

public:
   Viewer(QWidget* parent=0); //  Constructor
private slots:
   void setPlayText(QString text); // On Play/Pause
   void setProbabilityLabel(int t);
   void setIJKMinMax(int i,int j,int k);
   void loadFiles();
   void setProcessor();
   void updateSlider(int rangeMin, int rangeMax);
   void changeBackgroundColor();
   void saveAsPdf();
};

#endif
