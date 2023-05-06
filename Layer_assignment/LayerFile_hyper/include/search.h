#ifndef _SEARCH_H_
#define _SEARCH_H_


#include "structure.h"

void RandomTargetPd(NETLIST &nt, SEARCHINFO &searchInfo);
void SetGridSearchRange(SEARCHINFO &searchInfo, GRIDRANGE &gridRange);
void RecursiveGrid(GRIDRANGE &gridRange, int layer, double *layerPdArray, vector<SEARCHINFO> &searchInfoVec);
double CaculateSearchCost(IRBIN &IRBin, int* TSV_count );
int randint(int n);

#endif
