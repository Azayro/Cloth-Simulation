//
//  CGObject.cpp
//  CoreStructures
//

#include "CGObject.h"


CGObject::CGObject()
{
	retainCount = 1; //initialise retainCount - calling function adopts ownership
}


CGObject::~CGObject()
{
}


// retain object
void CGObject::retain()
{
	retainCount++;
}

// release ownership of object.  If retainCount=0 then delete object.
// Return true if the object is deleted successfully - so calling function knows
// if pointer/handle to object is valid or not.
bool CGObject::release()
{
	retainCount--;
	if (retainCount==0)
	{
		delete(this);
		return true;
	}

	return false;
}


void CGObject::report(FILE *fp)
{
	fp ? fprintf(fp, "retainCount=%d\n",retainCount) : printf("retainCount=%d\n",retainCount);
}


// Accessor methods

unsigned int CGObject::getRetainCount()
{
	return retainCount;
}


