//
//  RealBasicTestLib.c
//  RealBasicTestLib
//
//  Created by Ken Rorick on 1/7/12.
//  Copyright 2012 RORICK.NET. All rights reserved.
//

#include "RealBasicTestLib.h"

#include <string.h>
EXPORT int addFunction( int a, int b ) 
{ 
    return a + b;
}

EXPORT int stringLength( char *str ) 
{ 
    return strlen(str);
}
