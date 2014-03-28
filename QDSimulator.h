//
//  OpenGL QDModel Widget
//

#ifndef QDSIMULATOR_H
#define QDSIMULATOR_H
#include <vector>
#include "MarchingCubes.h"
#include <QGLWidget>
#include <QString>
#include <QColor>
using namespace std;

struct QDModel{
	int IMax;
	int JMax;
	int KMax;
	int states;
	mp4Vector * Points;
	bool hasProcessorInfo;
	double MaxProbability;
	double MinProbability;
	double *MinResidual;
	double *MaxResidual;
	int *iterationFileCount; 
	int processorCount;
	string directory;
};

class QDSimulator : public QGLWidget
{
Q_OBJECT                                             //  Qt magic macro
private:
   int state;
   int prevState;
   int eigenState;
   int prevEigenState;
   double x0,y0,z0;  //  Start position
   int    th,ph;     //  Display angles
   double dt;        //  Integration time step
   bool   mouse;     //  Mouse pressed
   QPoint pos;       //  Mouse position
   double dim;       //  Display size
   double asp;       //  Sceen aspect ratio
   QDModel qdModel;   // qdModel struct to hold prob data
   bool isPlay;       // play iterations
   bool isTransparent; // tranparency
   QTimer *idleTimer;  //timer
   int minProbabilityPower;   // minimum probability power  
   bool isAllIterations; // is  all iterations checkbox selected
   bool isResidual;  // is residual selected
   bool showCube; // show cube selected
   int iSlice,jSlice,kSlice; // gives the slice selected in th UI
   unsigned int prevTotalLines; // total lines read in the  last iteration
   bool * selectedProcessor; // gives the processors selected in the UI
   int iToDisplay,jToDisplay,kToDisplay; // I J K size to display
   bool isLoading;
   bool allProcessorsSelected;
   int currentProcessor;
   int totalProcessorsToDisplay;
   QColor *bgColor;
   
public:
   QDSimulator(QWidget* parent=0);                        //  Constructor
   bool loadFiles();
   bool setDirectory(QString dir);
   int getISlice();
   int getJSlice();
   int getKSlice();
   int getNumOfStates();
   bool hasProcessorInfo();
   int getProcessorCount();
   void setBackgroundColor(QColor* color);

public slots:
    void playIterations();
    void setState(int state);    //Slot set to state
    void setPlay();      //  Slot to play
    void setProbability(int prob);
    void setTransparency();
    void setAllIterations();
    void setResidual();
    void setShowCube();
    void setISlice(int i);
    void setJSlice(int j);
    void setKSlice(int k);
    void setDIM(double DIM);    //  Slot to set dim
    void reset(void);           //  Reset view angles
    void setSelectedProcessor(int processors[], int size);

signals:
    void angles(QString text);  //  Signal for display iteration numbers
    void play(QString txt);  //  Signal to change play button text
    void stateChanged(int st);
    bool onLoad(int i, int j, int k);
    void updateSliderRange(int min, int max);

protected:
    void initializeGL();                   //  Initialize widget
    void resizeGL(int width, int height);  //  Resize widget
    void paintGL();                        //  Draw widget
    void mousePressEvent(QMouseEvent*);    //  Mouse pressed
    void mouseReleaseEvent(QMouseEvent*);  //  Mouse released
    void mouseMoveEvent(QMouseEvent*);     //  Mouse moved
    void wheelEvent(QWheelEvent*);         //  Mouse wheel
private:
    void project();                        //  Set projection
    string getFileName();
    void getQDModelFromFile();             //  REads from binary file and sets QDModel
    void GetColorForProbability(double prob, double * r, double * g, double * b);
    void PaintLightSource();
    bool TextToBinary(string inputFileName, string outputFileName);
    void updateProbabilityRange();
};

#endif
