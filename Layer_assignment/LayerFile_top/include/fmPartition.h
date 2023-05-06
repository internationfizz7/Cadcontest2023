/*!
 * \file	fmPartition.h
 * \brief	fm partition
 *
 * \author	NCKU SEDA LAB WYChang
 * \version	0.1
 * \date	2018.09.13
 */
#ifndef _FMPARTITION_H_
#define _FMPARTITION_H_

#include <sstream>
#include <string>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <limits.h>
//#include "structure.cpp"
#include "structure.h"

#define AMP_PARA 1			///< constant to enlarge data (avoid rounding problem)
#define HARD_BLK '0'			///< hard module
#define SOFT_BLK '1'			///< soft module

using namespace std;

void initial_gain( NETLIST &nt , int tier );
void Bucket_List( NETLIST &nt , int tier );
void Thermal_bucket_list( NETLIST &nt , int tier);
bool cmp(int flag, int g, int area);
void Update_gain( NETLIST &nt , int i , int *modarea );
void Thermal_update_gain( NETLIST &nt , int tier , int *modarea);

void InitialThermalGain(NETLIST &nt, int tier);
void InitialTotalGain(NETLIST &nt, int tier);
void CalculatePowerDensity( NETLIST &nt );
void CalculatePower( NETLIST &nt );

#endif
