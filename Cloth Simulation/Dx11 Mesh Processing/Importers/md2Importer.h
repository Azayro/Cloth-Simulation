//
//  md2Importer.h
//

#pragma once

#pragma warning( disable : 4275 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4231 )

#include <CGImport3.h>

class CGModel;

CGIMPORT3_API CG_IMPORT_RESULT importMD2(wchar_t *filename, CGModel *model, int frame);


