#include <stdio.h>

typedef void (_stdcall *PF_LUT_Generator)(unsigned int uiIndex, unsigned char *pElement);
typedef void (_stdcall *PF_2DLUT_Generator)(unsigned int uiIndex1, unsigned int uiIndex2, unsigned char *pElement);
typedef void (_stdcall *PF_3DLUT_Generator)(unsigned int uiIndex1, unsigned int uiIndex2, unsigned int uiIndex3, unsigned char *pElement);
typedef void (_stdcall *PF_LUT_ElementFormatter)(FILE *fp, unsigned char *pElement);

typedef struct _tagLutDef
{
    int nTableDimension;
    int nTableSize[3];
    int nElementSize;
    PF_LUT_Generator pfGenerator;
    PF_2DLUT_Generator pf2DGenerator;
    PF_3DLUT_Generator pf3DGenerator;
    PF_LUT_ElementFormatter pfElementFormatter;
    const char *strName;
    const char *strElementTypeName;
} LUT_DEFINITION, *PLUT_DEFINITION;

#define BEGIN_LUT_DEFINITION \
    LUT_DEFINITION s_rgLutDefinitions[] = \
    {\
    

#define END_LUT_DEFINITION \
        {-1, {-1, -1, -1}, -1, NULL, NULL, NULL, NULL, NULL, NULL}\
    };

#define DEFINE_LUT(nTable1DSize, ElementType, pfGenerator, pfElementFormatter, strName) \
{1, {nTable1DSize, 0, 0}, sizeof(ElementType), pfGenerator, NULL, NULL, pfElementFormatter, strName, #ElementType},

#define DEFINE_2DLUT(nTable1DSize, nTable2DSize, ElementType, pfGenerator, pfElementFormatter, strName) \
{2, {nTable1DSize, nTable2DSize, 0}, sizeof(ElementType), NULL, pfGenerator, NULL, pfElementFormatter, strName, #ElementType},

#define DEFINE_3DLUT(nTable1DSize, nTable2DSize, nTable3DSize, ElementType, pfGenerator, pfElementFormatter, strName) \
{3, {nTable1DSize, nTable2DSize, nTable3DSize}, sizeof(ElementType), NULL, NULL, pfGenerator, pfElementFormatter, strName, #ElementType},

extern LUT_DEFINITION s_rgLutDefinitions[];

void _stdcall Lut_IntFormatter(FILE *fp, unsigned char *pElement);
void _stdcall Lut_ShortFormatter(FILE *fp, unsigned char *pElement);
void _stdcall Lut_Double(FILE *fp, unsigned char *pElement);
void _stdcall Lut_Float(FILE *fp, unsigned char *pElement);