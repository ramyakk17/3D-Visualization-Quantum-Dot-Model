//
//  Viewer Widget
//

#include <QtGui>
#include "viewer.h"
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

//
//  Constructor for Viewer widget
//
Viewer::Viewer(QWidget* parent)
    :  QWidget(parent)
{

   allProcessors = "All processors";
   resize(900,400);
   //  Set window title
   setWindowTitle(tr("Qt OpenGL Quantum dot simulator"));
   //  Create new QDSimulator widget
   qDSimulator = new QDSimulator;

   //Widgets
   state   = new QSpinBox();  
    //  Pushbutton to play all iterations
   play = new QPushButton("Play");
    //  Display widget for iteration 
   iteration = new QLabel();
    //  Pushbutton to reset view angle
   QPushButton* reset = new QPushButton("Reset display");
   // To select all iterations
   QCheckBox* allIterations = new QCheckBox("All iterations");
   //To select minimum probability
   probabilityLabel = new QLabel("Range min: 10^-7");
   minProbability = new QSlider(Qt::Horizontal);
    // To select all iterations
   QCheckBox* transparent = new QCheckBox("Transparent");
    // To display residual error
   residual = new QCheckBox("Residual");
    // To display cube
   QCheckBox* cube = new QCheckBox("Show cube/pyramid");
   //Load files
   QPushButton* loadFiles = new QPushButton("Load new files");
   QPushButton* changeBGColor = new QPushButton("Change Background color");
   QPushButton* saveSnapShot = new QPushButton("Export to pdf");
   saveFile = new QLineEdit();
   continueSameFile =false;
   //Label and spin box for slicing I,J,K
    QLabel* sliceILabel = new QLabel("Slice I");
    sliceI   = new QSpinBox(); 
    QLabel* sliceJLabel = new QLabel("Slice J");
    sliceJ   = new QSpinBox(); 
    QLabel* sliceKLabel = new QLabel("Slice K");
    sliceK   = new QSpinBox(); 

    processorLabel =new QLabel("Show processor");
    processorText = new QLineEdit();
    updateProcessorInfo();

    //  Set range and values
    state->setSingleStep(1.0);      state->setRange(0,qDSimulator->getNumOfStates()-1);          state->setValue(0);
    minProbability->setMinimum(-18); minProbability->setMaximum(-4); minProbability->setSingleStep(1); minProbability->setValue(-7); 
    minProbability->setTickPosition(QSlider::TicksBelow);  minProbability->setTickInterval(1);
    cube->setChecked(true);
    sliceI->setMinimum(0);
    sliceJ->setMinimum(0);
    sliceK->setMinimum(0);
    setIJKMinMax(qDSimulator->getISlice(), qDSimulator->getJSlice(), qDSimulator->getKSlice());
    saveFile->setText("output.pdf");

    //  Connect valueChanged() signals to qDModel slots 
   connect(state     , SIGNAL(valueChanged(int)) , qDSimulator , SLOT(setState(int)));
   //  Connect signals to display widgets
   connect(qDSimulator , SIGNAL(angles(QString)) , iteration , SLOT(setText(QString)));
   connect(play , SIGNAL(clicked(void))        , qDSimulator , SLOT(setPlay()));
   connect(qDSimulator , SIGNAL(play(QString)) , this , SLOT(setPlayText(QString)));
   connect(qDSimulator , SIGNAL(stateChanged(int)) , state , SLOT(setValue(int))); 
   connect(minProbability    , SIGNAL(valueChanged(int)) , this , SLOT(setProbabilityLabel(int)));
   connect(minProbability    , SIGNAL(valueChanged(int)) , qDSimulator , SLOT(setProbability(int)));
   connect(transparent,SIGNAL(stateChanged(int)) , qDSimulator , SLOT(setTransparency()));
   connect(allIterations,SIGNAL(stateChanged(int)) , qDSimulator , SLOT(setAllIterations()));
   connect(residual,SIGNAL(stateChanged(int)) , qDSimulator , SLOT(setResidual()));
   connect(cube,SIGNAL(stateChanged(int)) , qDSimulator , SLOT(setShowCube()));
   connect(reset , SIGNAL(clicked(void))        , qDSimulator , SLOT(reset()));
   connect(qDSimulator , SIGNAL(onLoad(int,int,int)) , this , SLOT(setIJKMinMax(int,int,int))); 
   connect(sliceI, SIGNAL(valueChanged(int)) , qDSimulator , SLOT(setISlice(int)));
   connect(sliceJ, SIGNAL(valueChanged(int)) , qDSimulator , SLOT(setJSlice(int)));
   connect(sliceK, SIGNAL(valueChanged(int)) , qDSimulator , SLOT(setKSlice(int)));
   connect(loadFiles,SIGNAL(clicked(void)) , this , SLOT(loadFiles()));
   connect(changeBGColor,SIGNAL(clicked(void)) , this , SLOT(changeBackgroundColor()));
   connect(saveSnapShot,SIGNAL(clicked(void)) , this , SLOT(saveAsPdf()));
   connect(qDSimulator, SIGNAL(updateSliderRange(int, int)), this, SLOT(updateSlider(int, int)));
   connect(processorText, SIGNAL(editingFinished()), this, SLOT(setProcessor()));

   //  Set layout of child widgets
   QGridLayout* layout = new QGridLayout;
   layout->setColumnStretch(0,100);
   layout->setColumnMinimumWidth(0,100);
   layout->setRowStretch(4,100);

   //  Simulator widget
   layout->addWidget(qDSimulator,0,0,5,1);

    //  Group State parameters
   QGroupBox* statebox = new QGroupBox("QD model parameters");
   QGridLayout* statelay = new QGridLayout;
   statelay->addWidget(new QLabel("State"),0,0);   statelay->addWidget(state,0,1);
   statelay->addWidget(iteration,1,1);             
   statelay->addWidget(play,2,0);            statelay->addWidget(allIterations,2,1);
   statelay->addWidget(probabilityLabel,3,0);   statelay->addWidget(minProbability,3,1); 
   statelay->addWidget(processorLabel,4,0);  statelay->addWidget(processorText,4,1);
   statelay->addWidget(residual,5,0);
  
   statebox->setLayout(statelay);
   layout->addWidget(statebox,0,1);

    //  Group display parameters
   QGroupBox* displaybox = new QGroupBox("Display options");
   QGridLayout* displaylay = new QGridLayout;
    
   displaylay->addWidget(transparent,1,0);
   displaylay->addWidget(cube,2,0);  
   displaylay->addWidget(sliceILabel,3,0); displaylay->addWidget(sliceI,3,1); 
   displaylay->addWidget(sliceJLabel,4,0); displaylay->addWidget(sliceJ,4,1); 
   displaylay->addWidget(sliceKLabel,5,0); displaylay->addWidget(sliceK,5,1); 

   displaylay->addWidget(reset,6,0);   displaylay->addWidget(changeBGColor,6,1);
   displaylay->addWidget(saveSnapShot,7,0); displaylay->addWidget(saveFile,7,1);
   displaylay->addWidget(loadFiles,8,0);  
   displaybox->setLayout(displaylay);
   layout->addWidget(displaybox,1,1);

   //  Overall layout
   setLayout(layout);
}

void Viewer::setIJKMinMax(int IMax, int JMax,int KMax)
{

	sliceI->setMaximum(IMax);
	sliceI->setValue(IMax);
	sliceJ->setMaximum(JMax);
	sliceJ->setValue(JMax);
	sliceK->setMaximum(KMax);
	sliceK->setValue(KMax);

}

 void Viewer::setPlayText(QString text)
{

	play->setText(text);
	QApplication::processEvents();

}

 void Viewer::setProbabilityLabel(int t)
{
	probabilityLabel->setText("Range min: 10^"+QString::number(t));
}

void Viewer::changeBackgroundColor()
{

   QColorDialog * backGroundColor = new QColorDialog(QColorDialog::ShowAlphaChannel);
   QColor* color = new QColor(backGroundColor->getColor());
   qDSimulator->setBackgroundColor(color);
}

void Viewer::saveAsPdf()
{
	if(!continueSameFile)
	{
		 painter.end();   
     		string directory="";
   		//Get the current directory this is for Linux
   		char buffer[FILENAME_MAX];
   		char* currentDir = GetCurrentDir(buffer,sizeof(buffer));
   		if(currentDir)
   		{
	   		string s = currentDir;
	   		directory = s+"//";
   		}
     		
     		QString filename = saveFile->text();
    		printer.setOutputFormat(QPrinter::PdfFormat);
     		if(!filename.contains(".pdf"))
         		filename = filename + ".pdf";     
     		printer.setOutputFileName(QString::fromStdString(directory)+filename);     		
     		if (! painter.begin(&printer)) { // failed to open file
         		qWarning("failed to open file, is it writable?");
         		return;
     		}
	}
     
     QRect applicationRect = qDSimulator->geometry();
     double aspectRatio = applicationRect.width()*1.0/ applicationRect.height();
     QRectF rectangle(10.0, 20.0, 500*aspectRatio, 500);
     painter.drawText(10, 10, "State :" + QString::number(state->value()) + "  " + iteration->text());     
     painter.drawPixmap(rectangle,QPixmap::grabWidget(qDSimulator), applicationRect);

     QMessageBox continueOrClose;
     continueOrClose.setText("Do you want to continue adding snapshots to same file?");
     continueOrClose.addButton("Continue", QMessageBox::YesRole);
     continueOrClose.addButton("Done", QMessageBox::NoRole);
     continueOrClose.exec();
     if(continueOrClose.result())
     {
	     painter.end();   
	    continueSameFile=false; 
     }
     else
     {
	     continueSameFile=true;
	     if (! printer.newPage()) 
         		qWarning("failed in flushing page to disk, disk full?");	     
     }
}

void Viewer::loadFiles()
{

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
	if(dir.isNull() || dir.isEmpty())
		return;
	bool fileLoaded=false;

	if(qDSimulator!=NULL)
		fileLoaded=qDSimulator->setDirectory(dir);

	if(!fileLoaded)
	{
		QMessageBox msgBox;
		msgBox.setText("Could not load files!");
		msgBox.exec();
	}
	else
	{
		//Update state information depending on newly loaded file
		state->setRange(0,qDSimulator->getNumOfStates()-1);          state->setValue(0);
		residual->setChecked(false);

		//update processor info
		updateProcessorInfo();
	}
}

//
//Update processor info on UI
//
void Viewer::updateProcessorInfo()
{
    if(qDSimulator->hasProcessorInfo())
    {
	processorText->show();
	processorLabel->show();
	processorText->setText(allProcessors.c_str());
	int numOfProcessors = qDSimulator->getProcessorCount()-1;
	processorLabel->setText("Show processor 0-" +QString::number(numOfProcessors) );
    }
    else
    {
	    processorText->hide();
	    processorLabel->hide();
    }

}

//
//  Set processor information
//
void Viewer::setProcessor()
{
	QString str =  processorText->text();

	int numOfProcessors = qDSimulator->getProcessorCount();

	if(str.contains(allProcessors.c_str()))
	{
		int * processorNumbers = new int[numOfProcessors];
		for(int i =0; i <numOfProcessors; i++)
			processorNumbers[i]=i;
			
		qDSimulator->setSelectedProcessor(processorNumbers, numOfProcessors);
	}
	else
	{
		/*//Validator for format 0,1,2,3,4..
    		QIntValidator validator = QIntValidator();
		validator.setRange(0,numOfProcessors);*/
		QStringList selectedProcessors = str.split(",", QString::SkipEmptyParts);
		int size =selectedProcessors.size();
		int p;
		int * processorNumbers = new int[size];
		for(int i =0; i < size; i++)
		{
			p=selectedProcessors[i].toInt();
			if(p<0 || p>=numOfProcessors)
			{
				QMessageBox msgBox;
                		msgBox.setText("Invalid processor numbers.");
                		msgBox.exec();
				break;
			}
                        processorNumbers[i] = selectedProcessors[i].toInt();
		}
		
		
		qDSimulator->setSelectedProcessor(processorNumbers, size);
	}

}

//
//  Update slider information
// 
//
void Viewer::updateSlider(int rangMin, int rangeMax)
{

minProbability->setMinimum(rangMin); 
minProbability->setMaximum(rangeMax); 

}
