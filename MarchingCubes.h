//Updated / modified by Ramya Nair
//
/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubes.cpp
//	Author	:	Michael Y. Polyakov
//	email	:	myp@andrew.cmu.edu  or  mikepolyakov@hotmail.com
//	website	:	www.angelfire.com/linux/myp
//	date	:	July 2002
//	
//	Description:	'Straight' and Recursive Marching Cubes Algorithms
//				Normal vectors are defined for each vertex as a gradients
//				For function definitions see MarchingCubes.h
//				For a tutorial on Marching Cubes please visit www.angelfire.com/myp/linux
//
//	Please email me with any suggestion/bugs.
/////////////////////////////////////////////////////////////////////////////////////////////


#include "mpVector.h"




//struct for storing triangle information - 3 vertices and 3 normal vectors for each vertex
typedef struct {
	mpVector p[3];
	mpVector norm[3];
} TRIANGLE;


//does Linear Interpolation between points p1 and p2 (they already contain their computed values)
mpVector LinearInterp(mp4Vector p1, mp4Vector p2, float value);


////////////////////////////////////////////////////////////////////////////////////////
//POINTERS TO FUNCTIONS
//pointer to function which computes the value at point p
typedef float (*FORMULA)(mpVector);
////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
///// MARCHING CUBES ALGORITHM /////
/////////////////////////////////////////////////////////////////////////////////////////

// 'STRAIGHT' - reduced Marching Cubes Algorithm///////////////////////////////////////
// to improve performance by avoiding multiplication by 1 
//takes number of cells (ncellsX, ncellsY, ncellsZ) to subdivide on each axis
// minValue used to pass into LinearInterp
// gradFactor for each axis (multiplies each component of gradient vector by 1/(2*gradFactor) ).
//		Should be set to the length of a side (or close to it)
// array of length (ncellsX+1)(ncellsY+1)(ncellsZ+1) of mp4Vector points containing coordinates and values
//returns pointer to triangle array and the number of triangles in numTriangles
//note: array of points is first taken on z axis, then y and then x. So for example, if you iterate through it in a
//       for loop, have indexes i, j, k for x, y, z respectively, then to get the point you will have to make the
//		 following index: i*(ncellsY+1)*(ncellsZ+1) + j*(ncellsZ+1) + k .
//		Also, the array starts at the minimum on all axes.

TRIANGLE* MarchingCubesReduced(int ncellsX, int ncellsY, int ncellsZ, float minValue, mp4Vector * points, int &numTriangles);
/////////////////////////////////////////////////////////////////////////////////////////


