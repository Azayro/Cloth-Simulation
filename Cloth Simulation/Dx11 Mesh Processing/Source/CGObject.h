//
//  CGObject.h
//  CoreStructures
//

#pragma once


#include <stdio.h>

// NULL pointer definition
#define	nil			0


// Define core abstract class
class CGObject
{
private:
	unsigned int			retainCount;

public:
	CGObject();
	// important - all derived classed have virtual destructors
	// This ensures appropriate destructor called for based-class pointer referenced sub-classes
	virtual ~CGObject();

	void retain();
	bool release();
	virtual void report(FILE *fp);

	// accessor methods
	unsigned int getRetainCount();
};



