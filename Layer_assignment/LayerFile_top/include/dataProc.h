/*!
 * \file	structure.cpp
 * \brief	data structure Parser
 *
 * \author	NCKU SEDA LAB PYChiu
 * \version	3.1
 * \date	2016.02.05
 */
#ifndef _DATAPROC_H_
#define _DATAPROC_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

//#include "structure.cpp"
#include "structure.h"

#define AMP_PARA 1			///< constant to enlarge data (avoid rounding problem)
#define HARD_BLK '0'			///< hard module
#define SOFT_BLK '1'			///< soft module

using namespace std;

void ReadBlockFile_HB(char *blockFile, NETLIST &nt);
void ReadNetFile_HB(char *netFile, NETLIST &nt);
void ReadPlFile_HB(char *plFile, NETLIST &nt);
void ReadPtFile_HB(char* ptFile, NETLIST& nt);

void read_GSRC_BLK( char *bench, NETLIST &nt );
void read_GSRC_NET( char *bench, NETLIST &nt );
void read_GSRC_PT( char *bench, NETLIST &nt );

#endif
