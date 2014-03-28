//
//  OpenGL Quantum dot model Widget
//
#include "QDSimulator.h"
#include "mpVector.h"
#include <QtOpenGL>
#include <iostream>
#include <fstream>
#include <limits>
#include <exception>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif



#include <QMessageBox>
#include <sstream>
#include <QColor>



using namespace std;

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.1415927/180))
#define Sin(x) (sin((x)*3.1415927/180))
//
//  Constructor
//
QDSimulator::QDSimulator(QWidget* parent)
    : QGLWidget(parent)
{
   th = -140;  //  Set intial display angles
   ph = 30;      
   dt  = 0.001;       //  Time step
   asp = 1;           //  Aspect ratio
   dim = 45;          //  World dimension
   x0 = y0 = z0 = 1;  //  Starting location
   mouse = 0;         //  Mouse movement
   state=0;           //State of electron
   prevState=-1;
   
   prevEigenState=-1;
   isPlay =false;
   isTransparent=false;
   isAllIterations =false;
   isResidual=false;
   showCube=true;
   // allocate some memory so it can be deleted
   qdModel.Points = new mp4Vector[1];
   qdModel.MaxResidual = new double[1];
   qdModel.MinResidual = new double[1];
   qdModel.iterationFileCount=new int[1];

   // timer for play funtionality
   idleTimer = new QTimer(this);
   connect(idleTimer, SIGNAL(timeout()), this, SLOT(playIterations()));
   idleTimer->start(5000); // increase this value to speed up
   idleTimer->blockSignals( true );
   minProbabilityPower = -7;
   iSlice=-1; 
   jSlice=-1; 
   kSlice=-1; 
   prevTotalLines =0;
   selectedProcessor =new bool[1];
   allProcessorsSelected=true;
   currentProcessor = -1;
   qdModel.directory="";
   //Get the current directory this is for Linux
   char buffer[FILENAME_MAX];
   char* currentDir = GetCurrentDir(buffer,sizeof(buffer));
   if(currentDir)
   {
	   string s = currentDir;
	   qdModel.directory = s+"//";
   }

   bgColor=new QColor();
   //bgColor.setRgbF(0.0,0.0,0.0);
   isLoading=false;
   loadFiles();

}

/********************************************************************/
/*************************  Set parameters  *************************/
/********************************************************************/

//
//  Set dt
//
void QDSimulator::setState(int State)
{
	state =State;
	if(!isPlay)
	eigenState=qdModel.iterationFileCount[state]-1;
	updateGL();   //  Request redisplay
}

//
//  Set iSlice
//
void QDSimulator::setISlice(int i)
{
   iSlice = i;
   updateGL();   //  Request redisplay
}

//
//  Set jSlice
//
void QDSimulator::setJSlice(int j)
{
   jSlice = j;
   updateGL();   //  Request redisplay
}

//
//  Set kSlice
//
void QDSimulator::setKSlice(int k)
{
   kSlice = k;
   updateGL();   //  Request redisplay
}

//
//  Set play
//
void QDSimulator::setPlay()
{
   isPlay =!isPlay;      //  Set parameter
   if(isPlay)
   {
	   idleTimer->blockSignals( false );
	   emit play("Pause");
   }
   else
   {
	   idleTimer->blockSignals( true );
	    emit play("Play");
   }
   //no need to update GL
}

//
//  Set dim
//
void QDSimulator::setDIM(double DIM)
{
   dim = DIM;    //  Set parameter
   project();
   updateGL();   //  Request redisplay
}

//
//  Play iterations
//
void QDSimulator::playIterations()
{
   if(isAllIterations)
   {
	if( eigenState+1<qdModel.iterationFileCount[state])
	   eigenState+=1;	  
   	else
   	{
      	   state+=1;
   	   if(state>qdModel.states-1)
	  	state =0;
	   eigenState=0;
   	   emit stateChanged(state);
        }
   }
   else
   {
	   state+=1;
   	   if(state>qdModel.states-1)
	  	state =0;
	   eigenState=qdModel.iterationFileCount[state]-1;
	   emit stateChanged(state);
   }
   updateGL();   //  Request redisplay
}

//
//  Set min probability
//
void QDSimulator::setProbability(int minProbPower)
{
   minProbabilityPower = minProbPower;
   updateGL();   //  Request redisplay
}

//
//  Set selected processor
//
void QDSimulator::setSelectedProcessor(int processorList[], int size)
{

	totalProcessorsToDisplay=0;
	for(int i=0;i<qdModel.processorCount;i++)
		selectedProcessor[i]=false;

 	for(int i=0;i<size;i++)
	{
		if(processorList[i]<qdModel.processorCount){
			selectedProcessor[processorList[i]]=true;
			totalProcessorsToDisplay++;
			
		}

	}
	//checks if all processors are selected for easier loops in getQDModelFromFile()
	//if(size>=qdModel.processorCount)
	{
		allProcessorsSelected = true;
		for(int i=0;i<qdModel.processorCount;i++)
		{
                	if(selectedProcessor[i]==false)
			{
				allProcessorsSelected=false;
				break;
			}
		}
	}

   updateGL();   //  Request redisplay
}

//
//  Set transparency
//
void QDSimulator::setTransparency()
{
   isTransparent =!isTransparent;      //  Set parameter
   updateGL();   //  Request redisplay
}

//
//  Set unset all iterations
//
void QDSimulator::setAllIterations()
{
   isAllIterations =!isAllIterations;      //  Set parameter
   //updateGL();   //  Request redisplay
}

//
//Set background color
//
void QDSimulator::setBackgroundColor(QColor* color)
{
  bgColor =color;     //  Set parameter
  updateGL();   //  Request redisplay
}

//
//  Set residual or not
//
void QDSimulator::setResidual()
{
   isResidual =!isResidual;      //  Set parameter
   updateGL();   //  Request redisplay
}

//
//  Set showCube
//
void QDSimulator::setShowCube()
{
   showCube=!showCube;
   updateGL();   //  Request redisplay
}

//
//  Set directory
//
bool QDSimulator::setDirectory(QString directory)
{
   qdModel.directory=directory.toStdString() + "//";
   return(loadFiles());   //   reading files
}

//
//  Reset view angle
//
void QDSimulator::reset(void)
{
   th = -140; //  Set parameter
   ph = 30; 
   dim = 45; 
   updateGL();   //  Request redisplay
}

/*
 * Reads text files while loading and converts them to binary
 * */
bool QDSimulator::TextToBinary(string inputFileName, string outputFileName)
{

    ifstream readFile (inputFileName.c_str(), ios::in);
    ofstream binaryFile (outputFileName.c_str(), ios::out|ios::binary|ios::ate);
    int partitionNumber;
    if(readFile.is_open() && binaryFile.is_open())
    {
     char text[5],x;
     int iTotal,jTotal,kTotal;
     string lineRead;
     bool isPartitioned=false;
     getline (readFile,lineRead);
     QString partitionInfo(lineRead.c_str());
     if(partitionInfo.contains("Yes"))
	isPartitioned =true;

     binaryFile.write(reinterpret_cast<const char *>(&(isPartitioned)), sizeof(bool));

     readFile>>text>>iTotal>>x>>jTotal>>x>>kTotal;
     binaryFile.write(reinterpret_cast<const char *>(&(iTotal)), sizeof(int));
     binaryFile.write(reinterpret_cast<const char *>(&(jTotal)), sizeof(int));
     binaryFile.write(reinterpret_cast<const char *>(&(kTotal)), sizeof(int));

     getline (readFile,lineRead);
     getline (readFile,lineRead);
     double *** probData = new double**[iTotal];  

     for(int i=0;i<iTotal;i++)
     {
        probData[i]  = new double*[jTotal];
        for(int j=0;j<jTotal;j++)
           probData[i][j]  = new double[kTotal];
     }

     unsigned int totalLines = iTotal*jTotal*kTotal;
     unsigned int counter =0;
     int iValue,jValue,kValue;

     for(counter=0;counter<totalLines;counter++)
     {
      if(readFile.eof())
      {
       cout<<"ERROR: While reading files , not enough lines"<<endl;
       break;
      }
      readFile>>iValue>>jValue>>kValue;
      readFile>>probData[iValue][jValue][kValue];
      if(isPartitioned)
	       readFile>>partitionNumber;

      getline(readFile,lineRead);
      binaryFile.write(reinterpret_cast<const char *>(&(iValue)), sizeof(int));
      binaryFile.write(reinterpret_cast<const char *>(&(jValue)), sizeof(int));
      binaryFile.write(reinterpret_cast<const char *>(&(kValue)), sizeof(int));
      binaryFile.write(reinterpret_cast<const char *>(&(probData[iValue][jValue][kValue])), sizeof(double));

      if(isPartitioned)
	      binaryFile.write(reinterpret_cast<const char *>(&(partitionNumber)), sizeof(int));

     }

     readFile.close(); 
     binaryFile.close();
    
    }
    else
    {
	    cout<<"ERROR: reading text files at "<< inputFileName<<endl;
	    QMessageBox msgBox;
	    msgBox.setText("ERROR: reading text files");
	    msgBox.exec();
	    return false;
    }
    
   return true;
}
//
//  load files => convert to binary, remember min max
//
bool QDSimulator::loadFiles()
{
	isLoading=true;
	string prefix = "eigenvector"; int j,i, states=-1; bool found =false;
	string filename; const int MAX_COUNTER=9;
	//resetting previous total lines
	prevTotalLines =0;

	double *maxResidual = new double[MAX_COUNTER];
	double *minResidual = new double[MAX_COUNTER];
	int *iterationFileCount = new int[MAX_COUNTER];
	for(int t=0;t<2;t++)
	{
		for(i=0;i<MAX_COUNTER;i++)
		{
			maxResidual[i] =0;
			minResidual[i] = numeric_limits<double>::max();
			state=i;
			for(j=0;j<MAX_COUNTER;j++)
			{
				std::ostringstream oss;
		                oss<<qdModel.directory<<prefix<<i<<"_iter"<<j;
				filename=oss.str()+".bin";
				ifstream readFileBin (filename.c_str(), ios::in);

				if(!readFileBin.good())
				{
					filename=oss.str()+".txt";
					ifstream readFileTxt (filename.c_str(), ios::in);
					if(readFileTxt.good())
				        {
						readFileBin.close();
						bool canRead = TextToBinary(oss.str()+".txt",oss.str()+".bin");
						if(!canRead)
							return false;
						found=true;
					}
					else
						break;
					
				}
				readFileBin.close();
				filename=oss.str()+".bin";
				ifstream readFileBinNew (filename.c_str(), ios::in);
				if(readFileBinNew.good())
				{

					found=true;
					readFileBinNew.close();
				
					if(t==1) //ie if residual files, read and record min max
					{		        
					   eigenState=j;
					   getQDModelFromFile();
					  
					   if(qdModel.MinProbability<minResidual[i])
						   minResidual[i]=qdModel.MinProbability;
					   if(qdModel.MaxProbability>maxResidual[i])
						   maxResidual[i]=qdModel.MaxProbability;

					}

				}

				if(!found)
				{
					cout<<"Error: Could not find file (neither text nor binary) "<< filename<<endl;
					return false;
				}
					

			}
			
			if(j==0)
			{
				if(states==-1)
					states = i;
				break;
			}
			iterationFileCount[i]=j;			
		}

		prefix = "residual";
		isResidual=true;
	}

	//Updating info related to number of states
	qdModel.states=states;
	delete [] qdModel.MaxResidual;
	delete [] qdModel.MinResidual;
	delete [] qdModel.iterationFileCount;

	qdModel.MaxResidual = new double[states];
	qdModel.MinResidual = new double[states];
	qdModel.iterationFileCount = new int[states];
	for(int k=0;k<states;k++)
	{
		qdModel.MaxResidual[k] =maxResidual[k];
		qdModel.MinResidual[k] =minResidual[k];
		qdModel.iterationFileCount[k] =iterationFileCount[k];
	}
	delete [] maxResidual;
	delete [] minResidual;
	delete [] iterationFileCount;


	qdModel.IMax=0;
       	qdModel.JMax=0;
	qdModel.KMax=0; 
	state=0;
	eigenState= qdModel.iterationFileCount[0]-1;
        isResidual=false;
        isLoading=false;
	updateGL();

	// now set Processor list as processorCount is now updated

	delete[] selectedProcessor;
        if(qdModel.hasProcessorInfo){
		selectedProcessor = new bool[qdModel.processorCount];
		allProcessorsSelected=true;
	}
	else
		selectedProcessor = new bool[1]; // dummy

	return found;
}

/******************************************************************/
/*************************  Get parameters  ***********************/
/******************************************************************/

int QDSimulator::getISlice()
{
	return iSlice;
}


int QDSimulator::getJSlice()
{
	return jSlice;
}

int QDSimulator::getKSlice()
{
	return kSlice;
}

int QDSimulator::getNumOfStates()
{
	return qdModel.states;
}

bool QDSimulator::hasProcessorInfo()
{
	return qdModel.hasProcessorInfo;
}

int QDSimulator::getProcessorCount()
{
	return qdModel.processorCount;
}

/******************************************************************/
/*************************  Mouse Events  *************************/
/******************************************************************/
//
//  Mouse pressed
//
void QDSimulator::mousePressEvent(QMouseEvent* e)
{
   mouse = true;
   pos = e->pos();  //  Remember mouse location
}

//
//  Mouse released
//
void QDSimulator::mouseReleaseEvent(QMouseEvent*)
{
    mouse = false;
}

//
//  Mouse moved
//
void QDSimulator::mouseMoveEvent(QMouseEvent* e)
{
   if (mouse)
   {
      QPoint d = e->pos()-pos;  //  Change in mouse location
      th = (th+d.x())%360;      //  Translate x movement to azimuth
      ph = (ph+d.y())%360;      //  Translate y movement to elevation
      pos = e->pos();           //  Remember new location
      updateGL();               //  Request redisplay
   }
}

//
//  Mouse wheel
//
void QDSimulator::wheelEvent(QWheelEvent* e)
{
   //  Zoom out
   if (e->delta()<0)
      setDIM(dim+10);
   //  Zoom in
   else if (dim>11)
      setDIM(dim-10);

}

/*******************************************************************/
/*************************  OpenGL Events  *************************/
/*******************************************************************/

//
//  Initialize
//
void QDSimulator::initializeGL()
{
   glEnable(GL_DEPTH_TEST); //  Enable Z-buffer depth testing
   setMouseTracking(true);  //  Ask for mouse events
}

//
//  Window is resized
//
void QDSimulator::resizeGL(int width, int height)
{
   //  Window aspect ration
   asp = (width && height) ? width / (float)height : 1;
   //  Viewport is whole screen
   glViewport(0,0,width,height);
   //  Set projection
   project();
}

//
// Gets file name corresponding to the state
//
string QDSimulator::getFileName()
{
   string filename;
   std::ostringstream oss;
   oss<<qdModel.directory;
   if(isResidual)
	oss<<"residual"<<state<<"_iter"<<eigenState<<".bin";
   else	//With eigen values:
   	oss<<"eigenvector"<<state<<"_iter"<<eigenState<<".bin";
   filename = oss.str();
    
   return filename;
}

//
// REads the binary file and returns QDModel object
//
void QDSimulator::getQDModelFromFile()
{ 

   string filename =getFileName();

    ifstream binaryTestFile (filename.c_str(), ios::in|ios::binary|ios::ate);
    int iOut,jOut,kOut;
    bool isPartitioned;
    int processorCount=-1;
 
   if (binaryTestFile.is_open())
   {

    binaryTestFile.seekg (0, std::ios::beg);
    binaryTestFile.read ((char*)&isPartitioned, sizeof(bool));
    binaryTestFile.read ((char*)&iOut, sizeof(int));   
    binaryTestFile.read ((char*)&jOut, sizeof(int));   
    binaryTestFile.read ((char*)&kOut, sizeof(int));   
 
     if(qdModel.IMax!=iOut || qdModel.JMax != jOut ||  qdModel.KMax != kOut)
     {
	     	iSlice = iOut-1;
		jSlice = jOut-1;
		kSlice = kOut-1;
     }

     
     if(isPartitioned && !allProcessorsSelected)
     {
	/*if(prevState==state && prevTotalLines==totalLines)
	   return;*/
	int filePosition=  binaryTestFile.tellg();

	if(qdModel.IMax!=iOut || qdModel.JMax != jOut ||  qdModel.KMax != kOut)
    	{
    		qdModel.IMax = iOut;
    		qdModel.JMax = jOut;
    		qdModel.KMax = kOut;
		if(!isLoading)
			emit onLoad(iOut-1,jOut-1,kOut-1);
    	}
	qdModel.hasProcessorInfo = true;

	unsigned int counter =0;
     	unsigned int totalCounter =iOut*jOut*kOut;
     	int iValue,jValue,kValue, partitionNumber;
     	int iMin=iOut,iMax=0,jMin=jOut,jMax=0,kMin=kOut,kMax=0;
     	double prob, maxProb=0, minProb=numeric_limits<double>::max();
   	//Read first time just to get I J K sizes
     	for(counter=0;counter<totalCounter;counter++)
     	{
      		binaryTestFile.read ((char*)&iValue, sizeof(int));  
      		binaryTestFile.read ((char*)&jValue, sizeof(int));    
      		binaryTestFile.read ((char*)&kValue, sizeof(int));      
      		binaryTestFile.read ((char*)&prob, sizeof(double));
		binaryTestFile.read ((char*)&partitionNumber, sizeof(int));
       	
		if(iValue>iSlice || jValue>jSlice || kValue>kSlice|| currentProcessor!=partitionNumber)//!selectedProcessor[partitionNumber])
			continue;
		if(iMin>iValue)
			iMin=iValue;
		if(jMin>jValue)
			jMin=jValue;
		if(kMin>kValue)
			kMin=kValue;
		if(iMax<iValue)
			iMax=iValue;
		if(jMax<jValue)
			jMax=jValue;
		if(kMax<kValue)
			kMax=kValue;		
	    	     
     	}
	int iSize =iMax-iMin+1;
	int jSize =jMax-jMin+1;
	int kSize =kMax-kMin+1;

	if(iSize<0||jSize<0||kSize<0)
	{
		cout<<"ERROR: Size of I, J , K selected is negetive";
		return;
	}

	unsigned int totalLines = iSize*jSize*kSize;

	if(prevTotalLines!=totalLines)
     	{
	    delete [] qdModel.Points;
	    qdModel.Points = new mp4Vector[totalLines];
    	}
	int JtimesK = jSize*kSize;
     	int kTotal = kSize;	
	binaryTestFile.seekg (filePosition, std::ios::beg);
	for(counter=0;counter<totalCounter;counter++)
     	{
      		binaryTestFile.read ((char*)&iValue, sizeof(int));  
      		binaryTestFile.read ((char*)&jValue, sizeof(int));    
      		binaryTestFile.read ((char*)&kValue, sizeof(int));      
      		binaryTestFile.read ((char*)&prob, sizeof(double));
		binaryTestFile.read ((char*)&partitionNumber, sizeof(int));
		if(processorCount<partitionNumber)
			processorCount= partitionNumber;
       		if(prob>maxProb)
	      		maxProb=prob;
      		if(prob<minProb)
	      		minProb=prob;
		if(iValue>iSlice || jValue>jSlice || kValue>kSlice|| currentProcessor!=partitionNumber)//!selectedProcessor[partitionNumber])
			continue;
	    	mp4Vector vert(iValue,jValue,kValue,prob);
      		qdModel.Points[((iValue-iMin)*JtimesK) + ((jValue-jMin)*kTotal) + (kValue-kMin)] = vert;  
     	}
    	qdModel.MaxProbability = maxProb;
    	qdModel.MinProbability = minProb;
    	qdModel.processorCount = processorCount+1;
	iToDisplay=iSize-1;
	jToDisplay=jSize-1;
	kToDisplay=kSize-1;
    	prevTotalLines = totalLines;
	if(!isLoading)
		updateProbabilityRange();

     }
     else
     {
     	unsigned int totalLines = (iSlice+1)*(jSlice+1)*(kSlice+1);
     	/*if(prevState==state && prevEigenState == eigenState && prevTotalLines==totalLines)
	   return;*/

     	if(prevTotalLines!=totalLines)
     	{
	    delete [] qdModel.Points;
	    qdModel.Points = new mp4Vector[totalLines];

    	}

   	if(qdModel.IMax!=iOut || qdModel.JMax != jOut ||  qdModel.KMax != kOut)
    	{
    		qdModel.IMax = iOut;
    		qdModel.JMax = jOut;
    		qdModel.KMax = kOut;
		if(!isLoading)
			emit onLoad(iOut-1,jOut-1,kOut-1);
    	}

    	qdModel.hasProcessorInfo = isPartitioned;

     	unsigned int counter =0;
     	unsigned int totalCounter =iOut*jOut*kOut;
     	int iValue,jValue,kValue, partitionNumber;
     	int JtimesK = (jSlice+1)*(kSlice+1);
     	int kTotal = (kSlice+1);
     	double prob, maxProb=0, minProb=numeric_limits<double>::max(); 
     	for(counter=0;counter<totalCounter;counter++)
     	{
      		binaryTestFile.read ((char*)&iValue, sizeof(int));  
      		binaryTestFile.read ((char*)&jValue, sizeof(int));    
      		binaryTestFile.read ((char*)&kValue, sizeof(int));      
      		binaryTestFile.read ((char*)&prob, sizeof(double));

		if(isPartitioned){
      			binaryTestFile.read ((char*)&partitionNumber, sizeof(int));
			if(processorCount<partitionNumber)
				processorCount= partitionNumber;
		}
		
       		if(prob>maxProb)
	      		maxProb=prob;
      		if(prob<minProb)
	      		minProb=prob;
      		if(iValue>iSlice || jValue>jSlice || kValue>kSlice)
			continue;
	    	mp4Vector vert(iValue,jValue,kValue,prob);
      		qdModel.Points[(iValue*JtimesK) + (jValue*kTotal) + kValue] = vert;       
	}
    	qdModel.MaxProbability = maxProb;
    	qdModel.MinProbability = minProb;

    	qdModel.processorCount = processorCount+1;
	iToDisplay=iSlice;
	jToDisplay=jSlice;
	kToDisplay=kSlice;

    	prevTotalLines = totalLines;
	if(!isLoading)
		updateProbabilityRange();
     }
     prevState = state;
     binaryTestFile.close();      
   }
   else
   {
	   cout<<"ERROR: Reading binary files. Filepath:"<<filename<<endl;;
   }
	   
  
}
void QDSimulator::GetColorForProbability(double prob, double * r, double * g, double * b)
{
 if(prob==qdModel.MinProbability)
 {
	 *r=0;
	 *g=0;
	 *b=0;
	 return;

 }
 const double minVisibleWaveLength = 450.0;
 const double maxVisibleWaveLength = 700.0;
 const double gamma = 0.9;
 double minValue = qdModel.MinProbability;
 double maxValue= qdModel.MaxProbability;
 if(isResidual)
 {
	 minValue = qdModel.MinResidual[state];
	 maxValue = qdModel.MaxResidual[state];
 }

 double scaled = (prob - minValue)/(maxValue-minValue);
 double wavelength = (scaled * (maxVisibleWaveLength - minVisibleWaveLength)) + minVisibleWaveLength;
 double red=0, blue=0, green=0;
  
  if(wavelength > 780) cout<<"Out of range: MaxValue"<<endl;
  else if(wavelength >=645){ red=1;blue=0;green=0;}
  else if(wavelength>=580){red=1;blue=0;green=-(wavelength - 645) / (645 - 580); }
  else if(wavelength>=510){ red=(wavelength - 510) / (580 - 510);blue=0;green=1;}
  else if(wavelength>=490){ red=0;blue = -(wavelength - 510) / (510 - 490); green =1;}
  else if(wavelength>=440){ red =0; blue =1; green = (wavelength - 440) / (490 - 440);}
  else if(wavelength>=380){ red = -(wavelength - 440) / (440 - 380); blue = 1; green=0;}
   
  *r = pow(red,gamma);
  *g = pow(green,gamma);
  *b = pow(blue,gamma);
}

void QDSimulator::PaintLightSource()
{
   glShadeModel( GL_SMOOTH);

   int local     =   0;  // Local Viewer Model
   int emission  =   50;  // Emission intensity (%)
   int ambient   =  30;  // Ambient intensity (%)
   int diffuse   = 100;  // Diffuse intensity (%)
   int specular  =   0;  // Specular intensity (%)

   /*double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ez = +2*dim        *Sin(ph);
   double Ey = +2*dim*Cos(th)*Cos(ph); */

   glPushMatrix();
  
   float yellow[] = {1.0,1.0,0.0,0.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    float shinyvec[] = { 20 };
   //  Translate intensity to color vectors
   float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
   float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
   float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
   float Position[]  =  {30,30,30,0.1};
 //emit angles("Ex:"+QString::number(Ex)+ "Ey " + QString::number(-Ey)+ "Ez " + QString::number(Ez));  
    glColor3f(1,1,1);
    //  Offset and scale
    glRotated(ph-90, 1,0,0);
    glRotated(th , 0,0,1);
    glTranslated(Position[0],Position[1],Position[2]);
 
    glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
    glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

    //  Undo transformations
      glPopMatrix();
   
   //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
}

//
//  Draw the window
//
void QDSimulator::paintGL()
{
/*QMessageBox msgBox;
msgBox.setText("ERROR: 1");
msgBox.exec();*/
//ofstream textFile ("test.txt", ios::out);
   PaintLightSource();

   //GET VALUES FROM FILE
   getQDModelFromFile();

   double r=0 , g=1, b=0;
   double minValue = qdModel.MinProbability;
   double maxValue = qdModel.MaxProbability;
   int iCells = qdModel.IMax-1;
   int jCells = qdModel.JMax-1;
   int kCells = qdModel.KMax-1;

      //  Clear screen and Z-buffer
   glClearColor(bgColor->redF(),bgColor->greenF(),bgColor->blueF(),bgColor->alphaF());
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //
   //  Reset transformations
   glLoadIdentity();

   glEnable(GL_LIGHTING);
   //DRAW MAIN OBJECT
    float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   float shinyvec[] = { 20 };
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    glEnable(GL_POINT_SMOOTH);
   glEnable(GL_BLEND);
    glLoadIdentity();
    glPushMatrix();
   glRotated(ph-90, 1,0,0);
   glRotated(th , 0,0,1);
   glTranslatef(-iCells/2,-jCells/2,-kCells/2);
   if(isTransparent)
   {
	glEnable(GL_BLEND);
   	glDepthMask(0);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   }
   else
   {
   	glDepthMask(1);
	glDisable(GL_BLEND );
   }  
   glBegin(GL_TRIANGLES);
   int triangleCount;
   TRIANGLE triangleObj;
   minValue = pow(10,minProbabilityPower);
   int totalProcessorLoops=1;
   if(qdModel.hasProcessorInfo && !allProcessorsSelected ){
	   totalProcessorLoops=totalProcessorsToDisplay;
   	   currentProcessor=-1;
   }
   for(int processorCount=0;processorCount<totalProcessorLoops;processorCount++){

   if(qdModel.hasProcessorInfo &&!allProcessorsSelected)
   {	   
	   currentProcessor++;
	   //update current processor for each loop
	   while(!selectedProcessor[currentProcessor])
		   currentProcessor++;
	    getQDModelFromFile();
	    
   }
   
    for(double topValue = maxValue;topValue>=minValue;topValue=topValue/50)
    {
	  double interval = topValue*9/50;
	  double newMin = topValue/50;
	   for(double value = topValue;value>=newMin;value-=interval)
	   {
   		TRIANGLE* triangles = MarchingCubesReduced(iToDisplay, jToDisplay, kToDisplay, value,qdModel.Points,triangleCount);	
   		GetColorForProbability(value,&r,&g,&b);

		double alpha =value/maxValue;
		if(isTransparent)
		{
			alpha =pow(value/maxValue,5);//value/maxValue;
			alpha=alpha>0.09? alpha:alpha+0.005;

		}
   		glColor4d(r,g,b, alpha);
       		glBegin(GL_TRIANGLES);
      
   		for(int i =0;i<triangleCount;i++)
   		{
		
			triangleObj = triangles[i];
			glNormal3f(-triangleObj.norm[0].x,triangleObj.norm[0].y,triangleObj.norm[0].z);
			glVertex3d(triangleObj.p[0].x,triangleObj.p[0].y,triangleObj.p[0].z);
			glNormal3f(-triangleObj.norm[1].x,triangleObj.norm[1].y,triangleObj.norm[1].z);
			glVertex3d(triangleObj.p[1].x,triangleObj.p[1].y,triangleObj.p[1].z);
			glNormal3f(-triangleObj.norm[2].x,triangleObj.norm[2].y,triangleObj.norm[2].z);
			glVertex3d(triangleObj.p[2].x,triangleObj.p[2].y,triangleObj.p[2].z);
   		}
		glEnd();
		delete [] triangles;
	
	}
   
   }
   }	
   glDisable(GL_BLEND);
   glDepthMask(1);
 
   //Done
   glPopMatrix();
   glDisable(GL_LIGHTING);

	
   //BOX
   
   if(showCube)
   {
   	int xLen = iCells+1;
   	int yLen =jCells+1;
   	int zLen = kCells+1;

      	//  Save transformation
   	glPushMatrix();
   	//  Offset, scale and rotate
   	glRotated(ph-90, 1,0,0);
   	glRotated(th , 0,0,1);
	glTranslatef(-iCells/2,-jCells/2,-kCells/2);
   	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

  
   	glEnable(GL_BLEND);
   	glDepthMask(0);
   	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glColor4d(1-bgColor->redF(),1-bgColor->greenF(),1-bgColor->blueF(),0.05);
   	//glColor4d(1,1,1,0.05);

   	//  Cube
   	glBegin(GL_QUADS);
   	//  Front
   	glNormal3f( 0, 0, 1);
   	glVertex3f(0,yLen,zLen);
   	glVertex3f(xLen,yLen,zLen);
   	glVertex3f(xLen,0,zLen);
   	glVertex3f(0,0,zLen);
   	//  Back
   	glNormal3f( 0, 0,-1);
 	glVertex3f(0,yLen,0);
   	glVertex3f(xLen,yLen,0);
   	glVertex3f(xLen,0,0);
   	glVertex3f(0,0,0);
   	//  Right
   	glNormal3f(+1, 0, 0);
   	glVertex3f(xLen,yLen,zLen);
   	glVertex3f(xLen,yLen,0);
   	glVertex3f(xLen,0,0);
   	glVertex3f(xLen,0,zLen);
   	//  Left
   	glNormal3f(-1, 0, 0);
   	glVertex3f(0,yLen,zLen);
   	glVertex3f(0,yLen,0);
   	glVertex3f(0,0,0);
  	glVertex3f(0,0,zLen);
   	//  Top
   	glNormal3f( 0,+1, 0);
   	glVertex3f(0,yLen,0);
   	glVertex3f(xLen,yLen,0);
   	glVertex3f(xLen,yLen,zLen);
   	glVertex3f(0,yLen,zLen);
   	//  Bottom
   	glNormal3f( 0,-1, 0);
   	glVertex3f(0,0,0);
   	glVertex3f(xLen,0,0);
   	glVertex3f(xLen,0,zLen);
   	glVertex3f(0,0,zLen);
   	//  End
   	glEnd();
   	//  Undo transofrmations
    	glDisable(GL_BLEND);
      	glDepthMask(1);
  
   	//BOX
   	glColor3f(1,1,1);
   
  	glBegin(GL_LINES);

   	//Cube
   	glVertex3d(0,0,0);  glVertex3d(xLen,0,0);
   	glVertex3d(xLen,0,0); glVertex3d(xLen,yLen,0);
   	glVertex3d(xLen,yLen,0);  glVertex3d(0,yLen,0);
   	glVertex3d(0,yLen,0); glVertex3d(0,0,0);

   	glVertex3d(0,0,zLen); glVertex3d(xLen,0,zLen);
   	glVertex3d(xLen,0,zLen); glVertex3d(xLen,yLen,zLen);
   	glVertex3d(xLen,yLen,zLen);  glVertex3d(0,yLen,zLen);
   	glVertex3d(0,yLen,zLen); glVertex3d(0,0,zLen);

   	glVertex3d(0,0,zLen); glVertex3d(0,0,0);
   	glVertex3d(0,yLen,zLen);  glVertex3d(0,yLen,0);
   	glVertex3d(xLen,yLen,zLen); glVertex3d(xLen,yLen,0);
   	glVertex3d(xLen,0,zLen); glVertex3d(xLen,0,0);
   	glEnd();
 
   
   	//pyramid
   	//flat part
   	glBegin(GL_LINES);
   	glColor3f(1,0,0);
	
	//Values obtained through email
	glVertex3d(15,15,15);  glVertex3d(15,47,15);
   	glVertex3d(15,47,15); glVertex3d(47,47,15);
   	glVertex3d(47,47,15); glVertex3d(47,15,15);
    	glVertex3d(47,15,15); glVertex3d(15,15,15);

	glVertex3d(15,15,15);  glVertex3d(31,31,31);
   	glVertex3d(15,47,15); glVertex3d(31,31,31);
   	glVertex3d(47,47,15); glVertex3d(31,31,31);
    	glVertex3d(47,15,15); glVertex3d(31,31,31);

   	glEnd();
  
   	glPopMatrix();
   }

   emit angles("Iteration:"+QString::number(eigenState)+ " of " + QString::number(qdModel.iterationFileCount[state]-1));

   //  Done
   glFlush();
}

//
//  Set projection
//
void QDSimulator::project()
{
   //  Orthogonal projection to dim
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (asp>1)
      glOrtho(-dim*asp, +dim*asp, -dim, +dim, -3*dim, +3*dim);
   else
      glOrtho(-dim, +dim, -dim/asp, +dim/asp, -3*dim, +3*dim);

   //  Back to model view
   glMatrixMode(GL_MODELVIEW);
}

/*
 * emits signal to update the slider ranges
 * */
void QDSimulator::updateProbabilityRange()
{
	double min = qdModel.MinProbability;
	double max = qdModel.MaxProbability;
	if(isResidual)
	{
		min = qdModel.MinResidual[state];
		max = qdModel.MaxResidual[state];
	}
	int minExp = log10(min);
	int maxExp = log10(max)-1;
	emit(updateSliderRange(minExp,maxExp));
}


