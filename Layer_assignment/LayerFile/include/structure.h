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
	char name[50];
	int  id;
	int  tier;

	PAD();
	~PAD();
};

class PIN
{
public:
	char name[50];
	int  id;
	int  corr_id;
	bool is_pad;

	PIN();
	~PIN();
};

class NET
{
public:
	char name[50];
	int  id;
	int  head;
	int  degree;

    int nTSV;				///< number of TSVs

    int group1_count;
    int group2_count;

	NET();
	~NET();
};


class Libcell  //4/26
{
public:
	char name[50];
	int id;
	int w;
	int h;
	int p;
	Libcell();
	~Libcell();
};
class Lib  //4/26
{
public:
	vector<Libcell> lib_cell;
	map<string, int> lib_NameToID;
	Lib();
	~Lib();
};
class MODULE
{
public:
	char name[50];
	int mod_w; //4/26
	int mod_h; //4/26
	
	int  id;
	int  tier;
	int  area;
	int  num_net;
	int  *array_net;

	double pd; ///<power density

	int nLayer; ///f///      ///< number of layers the stacked module with; 1: not stack, 2~: stack (different with nt.nLayer)

	char type;

	int gain;
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

    int nTSV;				///< number of TSVs
    int Layer;

    int nFile;             ///< For dump number of file

    int nHard;
    int nSoft;
    int totalModArea;


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

extern int format;

extern int metis_nparts;
extern int metis_ubfactor;
extern int metis_Nruns;
extern int metis_Rseed;

extern int max_mod_area;
extern double max_WS;
extern int nOutput;

extern unsigned short *buffNet;

extern int maxpin;
extern int maxarea;

#endif

