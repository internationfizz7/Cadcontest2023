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


#define HARD_BLK '0'			///< hard module
#define SOFT_BLK '1'			///< soft module

using namespace std;

class FMSOLUTION;

void initial_gain( NETLIST &nt , int tier );
void Bucket_List( NETLIST &nt , int tier );
void Thermal_bucket_list( NETLIST &nt , int tier);
void Thermal_connect_bucket_list( NETLIST &nt , int tier);
bool cmp(int flag, int g, int area);
bool cmpArea(int flag, int g, int area);
bool cmpPd(NETLIST& nt, int target_tier, int tier, double Pd);
void Update_gain( NETLIST &nt , int i , int *modarea );
void Thermal_update_gain( NETLIST &nt , int tier , int *modarea);
void Thermal_connect_update_gain( NETLIST &nt , int tier , int *modarea);

void InitialThermalConnectGain(NETLIST &nt, int tier);
void InitialThermalGain(NETLIST &nt, int tier);
void InitialTotalGain(NETLIST &nt, int tier);
void StoreSolution(NETLIST &nt, FMSOLUTION &sol);
void RestoreSolution(NETLIST &nt, FMSOLUTION &sol);
void CalculatePowerDensity( NETLIST &nt );
void CalculatePower( NETLIST &nt );
int CalculateConnectivity( NETLIST &nt );

class FMSOLUTION
{
    public:
        vector<int> module_tier;

        int connectivity;
};



#endif
