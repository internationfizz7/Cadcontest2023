/*!
 * \file	structure.h
 * \brief	data structures (header)
 *
 * \author	NCKU SEDA LAB
 * \version	1.0
 * \date	2015.04.05
 */

#ifndef _DATATYPE_H_
#define _DATATYPE_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>


#define FILELEN 100
#define MAX_INT 2147483647
#define MAX_FLT 3e+20
#define FIXED 0
#define MOVABLE -1
#define TSV_SIZE 3
#define TSV_AREA TSV_SIZE * TSV_SIZE

using namespace std;
//#include "structure.cpp"

class PAD
{
public:
	string name;
	int  id;
	int  tier;

	PAD();
	~PAD();
};

class PIN
{
public:
	string name;
	int  id;
	int  corr_id;
	bool is_pad;

	PIN();
	~PIN();
};

class NET
{
public:
	string name;
	int  id;
	int  head;
	int  degree;

    int nTSV;				///< number of TSVs

    int group1_count;
    int group2_count;

    int num_hot;

	NET();
	~NET();
};

class MODULE
{
public:
	string name;
	int  id;
	int  tier;
	int  area;
	int  num_net;
	int  *array_net;

	double pd; ///<power density
	bool hot_flag; ///<hot module or not

	int nLayer; ///f///      ///< number of layers the stacked module with; 1: not stack, 2~: stack (different with nt.nLayer)

	char type;

	int gain;
	int thermal_gain;
	int total_gain;
	int lock;

    MODULE& operator=(const MODULE &);

	MODULE();
	~MODULE();
};

class NETLIST
{
friend ostream& operator<<(ostream &, const NETLIST &);
public:
	int num_pad;
	vector<PAD> pads;

	int num_pin;
	vector<PIN> pins;

	int num_net;
	vector<NET> nets;

	int num_mod;
	vector<MODULE> mods;

	map<string, int> mod_NameToID;
	map<string, int> pad_NameToID;
	vector<double> tierpd_vector;
	vector<double> tierp_vector;

    int nTSV;				///< number of TSVs
    int Layer;

    int nFile;             ///< For dump number of file

    int nHard;
    int nSoft;
    int totalModArea;
    int nHot;

    int pd_mean;
    int pd_stddevi;

    double total_power;


	NETLIST();
	~NETLIST();
};

class Solution:public NETLIST{
//class Solution{
public:
    vector<MODULE> tmp_node;
//    int nTSV;
    double cost;
    int index;
    Solution *next;
    Solution *pre;
//    void Area();
//    Solution& operator=(const Solution &);
    Solution();
    ~Solution();
};

class gain
{
	public:
		gain* prev;
		gain* next;
		int cell_name;
};

extern int format;
extern int metis_nparts;
extern int metis_ubfactor;
extern int metis_Nruns;
extern int metis_Rseed;

extern int max_mod_area;
extern double max_WS;
extern int nOutput;
extern double top_Target;

extern unsigned short *buffNet;

extern int maxpin;
extern int maxarea;
extern int maxhotgain;


extern vector<gain*> store_gain;
extern vector<gain*> List;
extern vector<double> target_pd;


extern int targetArea;
extern int targetTSV;
extern int mod_constraint;
extern int hottest_mod_id;

#endif

