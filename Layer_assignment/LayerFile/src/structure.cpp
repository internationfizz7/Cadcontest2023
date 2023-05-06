/*!
 * \file	structure.cpp
 * \brief	data structure
 *
 * \author	NCKU SEDA LAB
 * \version	1.0
 * \date	2015.04.05
 */

#include <cstring>

using namespace std;

#include "structure.h"

PAD::PAD(): id(-1), tier(0)
{
	strcpy(name, "\0");
}

PAD::~PAD()
{ }

PIN::PIN(): id(-1), corr_id(-1), is_pad(false)
{
	strcpy(name, "\0");
}

PIN::~PIN()
{ }

NET::NET(): id(-1), head(-1), degree(0), nTSV(0), group1_count(0), group2_count(0)
{
	strcpy(name, "\0");
}

NET::~NET()
{ }

Lib::Lib()
{ }
Lib::~Lib()
{ }

Libcell::Libcell(): id(0) , w(0), h(0), p(0)
{ 
strcpy(name, "\0");
}
Libcell::~Libcell()
{ }
MODULE::MODULE(): mod_w(0), mod_h(0),id(-1), tier(0), area(0), num_net(0), array_net(NULL) , nLayer(1) , type(0), gain(0), lock(0)
{
	strcpy(name, "\0");
}

MODULE::~MODULE()
{ }

NETLIST::NETLIST(): num_pad(0), num_pin(0), num_net(0), num_mod(0), nTSV(0), Layer(0), nHard(0), nSoft(0),  totalModArea(0)
{ }

ostream& operator<<(ostream &output, const NETLIST &netlist)
{
	output << "***** Benchmark Characteristics *****" << endl;
	output << "-Number of modules         : " << netlist.num_mod << endl;
	output << "-(soft/hard)               : " << netlist.nSoft << "/" << netlist.nHard << endl;
	output << "-Number of pads            : " << netlist.num_pad << endl;
	output << "-Number of nets            : " << netlist.num_net << endl;
	output << "-Number of pins            : " << netlist.num_pin << endl;
	output << "-Number of layers          : " << netlist.Layer << endl;
	output << "-Total area of modules     : " << netlist.totalModArea << endl;
	output << "*************************************" << endl;
	return output;
}


MODULE& MODULE::operator=(const MODULE &module)
{
	if(&module != this)
	{
		strcpy(name, module.name);
		id = module.id;
		tier = module.tier;
		area = module.area;
		type = module.type;
	}
	return *this;
}

NETLIST::~NETLIST()
{ }

Solution::Solution(): cost(0) , index(0) , next(NULL) , pre(NULL)
{ }

Solution::~Solution()
{
    next = NULL;
    pre  = NULL;
    tmp_node.clear();
}

/*
void Solution::Area(){
    int *Area=new int[Layer];
}*/



int format = 0;
int metis_nparts = 0;
int metis_ubfactor = 0;
int metis_Nruns = 0;
int metis_Rseed = 0;

int max_mod_area = 0;
double max_WS = 0.0;
unsigned short *buffNet;
int nOutput = 0;

int maxpin=0;
int maxarea=0;
