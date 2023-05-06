/*!
 * \file    main.cpp (layer assignment)
 * \brief   user interface & function interface
 *
 * \author  NCKU SEDA LAB PYChiu
 * \version 3.1
 * \date    2016.02.05
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;
using std::random_shuffle;

#include "structure.h"
#include "dataProc.h"
#include "fmPartition.h"




///---- exponential table ----//
//#define EXP(x) ExpTable[ (int)(1000*x) + 700000 ]	///< exponential conversion
//double ExpTable[1400001];			///< exponential table

// hmetis
extern "C"
{
#include <hmetis.h>
}

bool in_MAX_AREA( NETLIST &nt );

void assign_TIER( NETLIST &nt );
void dump_TIER( string bench, NETLIST &nt, bool success, char *addName, bool has_addName ,class Solution *ptr , int nFile);
void dump_Block( char *bench, NETLIST &nt, bool success, char *addName, bool has_addName);
void CalculateArea( NETLIST &nt );
int CalculateTSV( NETLIST &nt,int *TSV_count);
void TSVCount( NETLIST &nt , int *TSV_count );
void get_area_plusTSV( NETLIST &nt , int *modarea , int *TSV_count);
void get_area_plusTSV_sol( NETLIST &nt , Solution &sol ,int *modarea , int *TSV_count);
int TSVCount_sol( NETLIST &nt , Solution &sol , int *TSV_count);
int TestBench_StackedModule(NETLIST &nt);///f///create a benchmark for stacked module
void findHotModule(NETLIST &nt, double hot_rate);
void calculatePdStdDevi(NETLIST &nt); ///<calculate standard deviation of power.


int main( int argc, char **argv )
{
	char bench[FILELEN] = "\0";		///< benchmark name (including directory path)
	char str_format[4] = "1";		///< benchmark format. ("1": GSRC)

	char str_nparts[4] = "2";		///< number of desired partitions (equal to number of tiers)
	char str_ubfactor[4] = "1"; 	///< hmetis UBfactor
	char str_Nruns[4] = "10";		///< hmetis Nruns
	char str_Rseed[4] = "-1";		///< hmetis initial random seed
	char str_maxWS[4] = "15";
	char str_addName[4] = "-1";
	char str_output[4] = "1";      ///< output the best N result of all SA solution
	char str_topTargetPd[5] = "0.6"; ///the power density of the top layer
	double hot_rate = 0.3;
	string OutputFile  = "";

	bool is_appointed[4] =			///< flags for checking parameters has been appointed
	{
		false,	///< [0] benchmark format
		false,	///< [1] benchmark name
		false,	///< [2] help
		false	///< [3] additional name
	};

	argv++;
	argc--;

	int num_command = argc;

	while( argc )
	{
		if( !strcmp( *argv, "-format" ) )
		{
			argv++;
			argc--;
			strcpy( str_format, *argv );
			is_appointed[0] = true;
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-bench" ) )
		{
			argv++;
			argc--;
			strcpy( bench, *argv );
			is_appointed[1] = true;
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-help" ) )
		{
			argv++;
			argc--;
			is_appointed[2] = true;
		}
		else if( !strcmp( *argv, "-nparts" ) )
		{
			argv++;
			argc--;
			strcpy( str_nparts, *argv );
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-toptarget" ) )
		{
			argv++;
			argc--;
			strcpy( str_topTargetPd, *argv );
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-ubfactor") )
		{
			argv++;
			argc--;
			strcpy( str_ubfactor, *argv );
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-nruns") )
		{
			argv++;
			argc--;
			strcpy( str_Nruns, *argv );
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-rseed" ) )
		{
			argv++;
			argc--;
			strcpy( str_Rseed, *argv );
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-maxWS") )
		{
			argv++;
			argc--;
			strcpy( str_maxWS, *argv );
			argv++;
			argc--;
		}
        else if( !strcmp( *argv, "-hotrate") )
		{
            char str_hot[6] = "";
			argv++;
			argc--;
			hot_rate = atof(strcpy( str_hot, *argv ));
			argv++;
			argc--;
		}
		else if( !strcmp( *argv, "-addName") )
		{
			argv++;
			argc--;
			strcpy( str_addName, *argv );
			is_appointed[3] = true;
			argv++;
			argc--;
		}
        else if( !strcmp( *argv, "-output") )
		{
			argv++;
			argc--;
			strcpy( str_output, *argv );
			argv++;
			argc--;
		}
		else if(!strcmp(*argv, "-OutputFile"))
		{

			argv++;
			argc--;
			OutputFile = *argv;

			argv++;
			argc--;
		}
		else
		{
			argv++;
			argc--;
		}
	}

	if( num_command == 0 || !is_appointed[0] || !is_appointed[1] || is_appointed[2] )
	{
		cout << "                                                                                  " << endl;
		cout << "                      Thermal Layer Assignment - Version 0.2                          " << endl;
		cout << "                                 by NCKU SEDA LAB                                 " << endl;
		cout << "                                                                                  " << endl;
		cout << "       basic usage: ./LA -format <benchmark_format> -bench <benchmark_name>       " << endl;
		cout << "                                                                                  " << endl;
		cout << " arguments:                                                                       " << endl;
		cout << "   -format    <benchmark_format>  format of benchmark (\"1\": GSRC)                " << endl;
		cout << "   -bench     <benchmark_name>    name of benchmark including directory path       " << endl;
		cout << "   -nparts    <number>            number of partitions                             " << endl;
		cout << "   -ubfactor  <number>            hmetis UBfactor (\"1\" for default)              " << endl;
		cout << "   -nruns     <number>            hmetis Nruns (\"1\" for default)                 " << endl;
		cout << "   -rseed     <number>            hmetis initial random seed (\"-1\" for default)  " << endl;
		cout << "   -addName   <name>              additional name of dump file                     " << endl;
        cout << "   -output    <number>            output the best N result of all SA solution      " << endl;
		cout << "   -toptarget <number>            the power density of the top layer              " << endl;
		cout << "   -help                          help message and exit                           " << endl;
		cout << "                                                                                  " << endl;
		exit(EXIT_FAILURE);
	}

	format = atoi( str_format );
	metis_nparts = atoi( str_nparts );
	metis_ubfactor = atoi( str_ubfactor );
	metis_Nruns = atoi( str_Nruns );
	metis_Rseed = atoi( str_Rseed );
	max_WS = 0.01 * atoi( str_maxWS );
	nOutput = atoi( str_output );
	top_Target = atof( str_topTargetPd );



	cout << "-OutputFile : "<<OutputFile<<endl;
	cout << "-format  : " << format << endl;
	cout << "-nparts  : " << metis_nparts << endl;
	cout << "-ubfactor: " << metis_ubfactor << endl;
	cout << "-nruns   : " << metis_Nruns << endl;
	cout << "-rseed   : " << metis_Rseed << endl;
	cout << "-toptarget  : " << top_Target << endl;
	cout << "-maxWS   : " << max_WS << endl;
    cout << "-output  : " << nOutput << endl;

	NETLIST nt;
    clock_t start_t1 ,end_t1;     ///y///1009
    start_t1=clock();
	switch( format )
	{
		case 1:
			read_GSRC_BLK( bench, nt );
			read_GSRC_NET( bench, nt );
			//read_GSRC_PT( bench, nt );
			ReadPtFile_HB(bench, nt);
			break;
        case 2:                                            /// For ibm HB/HB+
            cout<<"[Info] Read the Block file..."<<endl;
            ReadBlockFile_HB(bench, nt);
            cout<<"[Info] Read the Net file..."<<endl;
            ReadNetFile_HB(bench, nt);
            cout<<"[Info] Read the Pl file..."<<endl;
            ReadPlFile_HB(bench, nt);
            cout<<"[Info] Read the Pt file..."<<endl;
            ReadPtFile_HB(bench, nt);
            cout<<"[Info] Read file Succeed"<<endl;
            break;
		default:
			cout << "error  : format " << format << " doesnt's exist!" << endl;
			exit(EXIT_FAILURE);
			break;
	}
	nt.Layer=metis_nparts;
	target_pd.resize(metis_nparts);
	nt.tierp_vector.resize(metis_nparts);
    cout<<nt<<endl;
	bool success = false;
	int num_iter = 0;

	for(int i=0; i<nt.num_mod; i++)
        nt.totalModArea+=nt.mods[i].area;

    ///set target pd
    CalculatePower(nt);
    nt.total_power = 0;
    double each_layer_area = (double)nt.totalModArea/(double)nt.Layer;
    //cout << "nt.totalModArea: " << nt.totalModArea << endl;
    //cout << "each_layer_area: " << each_layer_area << endl;
    cout << "Target pd:" << endl;
    for(int i=0; i<metis_nparts; i++)
        nt.total_power += nt.tierp_vector[i];

    target_pd[nt.Layer-1] = (nt.total_power*top_Target)/each_layer_area;
    for(int i=0; i<metis_nparts-1; i++)
        target_pd[i] = (nt.total_power*( (1-top_Target)/ (double)(nt.Layer-1))) / each_layer_area;

    for(int i=0; i<metis_nparts; i++)
        cout << "\tlayer " << i << " : " << target_pd[i] << endl;
	///Calculate the XL module number in ibm case
	//vector<int> large;
/*
	int XLMod_num=0,LMod_num=0;
    int avgArea=nt.totalModArea/nt.num_mod;
    long int LargeArea=nt.totalModArea/metis_nparts;
	for(int i=0;i<nt.num_mod;i++)
	{
        if(nt.mods[i].area > LargeArea)
        {
            XLMod_num++;
            //large.push_back(i);
            //cout<<nt.mods[i].area<<endl;
        }
        if(nt.mods[i].area > avgArea )
            LMod_num++;

	}
	cout<<"LargeArea = "<<LargeArea<<" XLMod_num = "<<XLMod_num<<endl;
	cout<<"avgArea = "<<avgArea<<"  LMod_num = "<<LMod_num<<endl;
*/
    //for(unsigned int i=0;i<large.size();i++)
        //cout<<large[i]<<endl;
    ///=====hNetis=====///
	assign_TIER( nt );
    cout<<"[Info] Initial Partition Success"<<endl;
	success = in_MAX_AREA( nt );

	while( !success && num_iter <= 10 )
	{
		assign_TIER( nt );
		success = in_MAX_AREA( nt );

		if( !success )
		{
//			metis_Rseed = (metis_Rseed + 1) % 1000;
            metis_Rseed = (metis_Rseed + 1);
		}

		num_iter++;
	}
	findHotModule(nt, hot_rate);
    int *mod_area = new int[ metis_nparts ]();
    int *TSV_count = new int[ metis_nparts - 1 ];
//    TSVCount(nt,TSV_count);
    get_area_plusTSV( nt , mod_area , TSV_count);
    cout<<"===========Initial State==========="<<endl;
    for(int i=0;i<metis_nparts - 1 ;i++){
        cout<<"Tier= "<<i<<"  TSV="<<TSV_count[i]<<endl;
    }
    targetTSV=CalculateTSV( nt ,TSV_count);
    cout<<"TSV#     : "<<targetTSV<<endl;
    for(int i=0;i<metis_nparts ;i++){
        targetArea+=mod_area[i];
        cout<<"Tier= "<<i<<"  ModuleArea="<<mod_area[i]<<endl;
    }
    //targetTSV=(int)(targetTSV*0.01);
    targetArea+=targetTSV*TSV_AREA;
    targetArea/=metis_nparts;
    mod_constraint=0;
    for(int i=0;i<metis_nparts;i++)
    {
        if(abs(mod_area[i]-targetArea)>mod_constraint)
            mod_constraint=abs(mod_area[i]-targetArea);
    }
    cout<<"module area constraint:"<<mod_constraint<<endl;
    cout<<"target Area:"<<targetArea<<" increasing TSV: "<<targetTSV<<endl;

    cout<<"==================================="<<endl;
    ///  show module connect to which nets
/*
    for(int i=0;i<nt.num_mod;i++)
    {
        cout<<nt.mods[i].name<<"  "<<nt.mods[i].num_net<<":   ";
        for(int j=0;j<nt.mods[i].num_net;j++)
        {
            cout<<nt.mods[i].array_net[j]<<"  ";
        }
        cout<<endl;
    }
*/
    ///====calculate hot module number in net====///
    for(int i=0; i<nt.num_net;i++)
        for(int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++)
            if(nt.mods[ nt.pins[j].corr_id ].hot_flag && nt.pins[j].is_pad ==0)
                nt.nets[i].num_hot++;

    nt.tierpd_vector.resize(nt.Layer);

    ///===thermal partition===========///
    cout << "[Info] thermal partitio start" << endl;
    cout<<targetArea*1.01<<"  "<<targetArea*0.99<<endl;
    for(int con=0; con<nt.Layer-1; con++)
    {
        for(int i=0;i<metis_nparts-1;i++)
        {
            CalculatePowerDensity(nt);
            InitialThermalGain(nt,i);
            initial_gain(nt,i);
            InitialTotalGain(nt,i);
            Thermal_bucket_list(nt, i);
            Thermal_update_gain(nt,i,mod_area);

            for(int j=0;j<nt.num_mod;j++)
            {
                nt.mods[j].gain=0;
                nt.mods[j].thermal_gain=0;
                nt.mods[j].total_gain=0;
            }
            List.clear();
            store_gain.clear();
            CalculatePowerDensity(nt);
        }
    }
    vector<int> hot_count_vec;
    hot_count_vec.resize(3);
    for(int i=0;i<metis_nparts ;i++){
        cout<<"Tier= "<<i<<"  ModuleArea="<<mod_area[i]<<endl;
    }
    for(int i=0;i<nt.num_mod;i++)
    {
        if(nt.mods[i].hot_flag)
            hot_count_vec[nt.mods[i].tier]++;
    }
    for(int i=0;i<metis_nparts;i++)
    {
        cout << "hot_num in tier " << i << " : " << hot_count_vec[i] << endl;
    }

    targetArea =0;
    for(int i=0;i<metis_nparts ;i++){
        targetArea+=mod_area[i];
        cout<<"Tier= "<<i<<"  ModuleArea="<<mod_area[i]<<endl;
    }
    //targetTSV=(int)(targetTSV*0.01);
    targetArea+=targetTSV*TSV_AREA;
    targetArea/=metis_nparts;
    targetArea*=0.95;
    mod_constraint=0;
    for(int i=0;i<metis_nparts;i++)
    {
        if(abs(mod_area[i]-targetArea)>mod_constraint)
            mod_constraint=abs(mod_area[i]-targetArea);
        cout << mod_area[i] << " " << targetArea << endl;
    }

    cout<<"module area constraint:"<<mod_constraint<<endl;
    cout<<"target Area:"<<targetArea<<" increasing TSV: "<<targetTSV<<endl;
    cout<<"[Info] thermal partition success" << endl;
    cout<<"[Info] normal partition start" << endl;

    for(int i=0;i<metis_nparts-1;i++)
    {
        //CalculatePowerDensity(nt);
        //InitialThermalGain(nt,i);
        initial_gain(nt,i);
        //InitialTotalGain(nt,i);
        Bucket_List(nt,i);

        Update_gain( nt , i , mod_area );


        //cout<<"***************************************"<<endl;

        //for(int j=0;j<nt.num_mod;j++)
        //    cout<<nt.mods[j].name<<" "<<nt.mods[j].tier<<"   "<<nt.mods[j].gain<<endl;

        for(int j=0;j<nt.num_mod;j++)
            nt.mods[j].gain=0;
        List.clear();
        store_gain.clear();
    }
    cout<<"[Info] normal partition success" << endl;

    cout<<"===========Without TSV==========="<<endl;
    for(int i=0;i<metis_nparts ;i++){
        cout<<"Tier= "<<i<<"  ModuleArea="<<mod_area[i]<<endl;
    }

    get_area_plusTSV( nt , mod_area , TSV_count);
    cout<<"==========Calculate TSV=========="<<endl;
    for(int i=0;i<metis_nparts - 1 ;i++){
        cout<<"Tier= "<<i<<"  TSV="<<TSV_count[i]<<endl;
    }
    targetTSV=CalculateTSV( nt ,TSV_count);
    cout<<"TSV#     : "<<targetTSV<<endl;
    CalculatePowerDensity(nt);
    for(int i=0;i<metis_nparts ;i++){
        cout<<"Tier= "<<i<<"  ModuleArea="<<mod_area[i]<<endl;
    }
    for(int i=0; i<nt.Layer; i++)
    {
        cout <<"tier " << i << " pd: " << nt.tierpd_vector[i] << endl;
    }




    nt.nTSV=CalculateTSV( nt ,TSV_count);

    nt.nFile = nOutput;

    class Solution *ptr= new class Solution;
    for(int i=0;i<nt.num_mod;i++)
    {
        ptr->tmp_node.push_back(nt.mods[i]);
    }
    ptr->pre=ptr;
    ptr->index=0;
    get_area_plusTSV( nt , mod_area , TSV_count);

    end_t1=clock();
    double rtime=(double)((end_t1-start_t1)/(double)CLOCKS_PER_SEC);
    cout<<"Rseed="<<metis_Rseed<<endl;
    cout<<"RunTime= "<<rtime<<endl;
    while(ptr!= ptr->pre){
        ptr=ptr->pre;
    }
    class Solution *dptr = ptr;
    while(dptr->next!=NULL)
    {
        dptr=dptr->next;
    }
//*/
    for(int i=0; i<nt.nFile; i++)
    {
        if(!success)
        {
            cout<<"SA Fail Dump origin partition result...."<<endl;
        }
        cout<<"Dump Layer file"<<i;
        success=true;
        dump_TIER( OutputFile, nt, success, str_addName, is_appointed[3] , dptr , i );
        cout<<"\tindex = "<<dptr->index<<"\t#TSV = "<<TSVCount_sol( nt , *dptr , TSV_count)<<endl;
        dptr=dptr->pre;
    }
    ///y///for stack mod
//    dump_Block( bench, nt, success, str_addName, is_appointed[3]);
//     modIndex.clear();
//	delete [] mod_area;
//	delete [] TSV_count;
	return 0;
}

void TSVCount( NETLIST &nt , int *TSV_count)
{
    for(int i=0; i<nt.Layer-1; i++)
        TSV_count[i]=0;

    for( int i = 0; i < nt.num_net; i++ )
	{
		int max_tier = 0;
		int min_tier = MAX_INT;

		for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
		{
			if( nt.pins[j].is_pad )
			{
				min_tier = 0;
				continue;
			}

//			if( min_tier > nt.mods[ nt.pins[j].corr_id ].tier )
//				min_tier = nt.mods[ nt.pins[j].corr_id ].tier;
			if( min_tier > nt.mods[ nt.pins[j].corr_id ].tier + nt.mods[ nt.pins[j].corr_id ].nLayer -1 )
				min_tier = nt.mods[ nt.pins[j].corr_id ].tier + nt.mods[ nt.pins[j].corr_id ].nLayer -1 ;
//			if( max_tier < nt.mods[ nt.pins[j].corr_id ].tier + nt.mods[ nt.pins[j].corr_id ].nLayer -1  )
//				max_tier = nt.mods[ nt.pins[j].corr_id ].tier + nt.mods[ nt.pins[j].corr_id ].nLayer -1 ;
            if( max_tier < nt.mods[ nt.pins[j].corr_id ].tier   )
				max_tier = nt.mods[ nt.pins[j].corr_id ].tier ;
		}

		if( (max_tier-min_tier) <= 0 ) continue;

		for( int j = 0 ; j < (max_tier-min_tier); j++ )  //j=0
		{
			TSV_count[j]++;
		}
	}
}

int TSVCount_sol( NETLIST &nt , Solution &sol , int *TSV_count)
{
    int nTSV=0;
    for(int i=0; i<nt.Layer-1; i++)
        TSV_count[i]=0;

    for( int i = 0; i < nt.num_net; i++ )
	{
//		int max_tier = 0;
//		int min_tier = MAX_INT;
        int max_tier = -1;
		int min_tier = nt.Layer;

		for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
		{
			if( nt.pins[j].is_pad )
			{
				min_tier = 0;
				continue;
			}

//			if( min_tier > sol.tmp_node[ nt.pins[j].corr_id ].tier )
//				min_tier = sol.tmp_node[ nt.pins[j].corr_id ].tier;
            if( min_tier > sol.tmp_node[ nt.pins[j].corr_id ].tier + sol.tmp_node[ nt.pins[j].corr_id ].nLayer - 1  )
				min_tier = sol.tmp_node[ nt.pins[j].corr_id ].tier + sol.tmp_node[ nt.pins[j].corr_id ].nLayer - 1;
//			if( max_tier < sol.tmp_node[ nt.pins[j].corr_id ].tier + sol.tmp_node[ nt.pins[j].corr_id ].nLayer -1 )
//				max_tier = sol.tmp_node[ nt.pins[j].corr_id ].tier + sol.tmp_node[ nt.pins[j].corr_id ].nLayer -1 ;
            if( max_tier < sol.tmp_node[ nt.pins[j].corr_id ].tier )
				max_tier = sol.tmp_node[ nt.pins[j].corr_id ].tier ;
		}

		if( (max_tier-min_tier) <= 0 ) continue;

		for( int j = 0 ; j < (max_tier-min_tier); j++ )  //j=0
		{
			TSV_count[j]++;
		}
	}
	for(int k=0; k < nt.Layer-1 ;k++)
        nTSV+=TSV_count[k];
    return nTSV;
}

bool in_MAX_AREA( NETLIST &nt )
{
	int *mod_area = new int[ metis_nparts ]();
	for( int i = 0; i < nt.num_mod; i++ )
	{
		mod_area[ nt.mods[i].tier ] += nt.mods[i].area;
	}

	int *TSV_count = new int [ metis_nparts-1 ]();
	for( int i = 0; i < nt.num_net; i++ )
	{
		int max_tier = 0;
		int min_tier = MAX_INT;

		for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
		{
			if( nt.pins[j].is_pad )
			{
				min_tier = 0;
				continue;
			}

			if( min_tier > nt.mods[ nt.pins[j].corr_id ].tier )
				min_tier = nt.mods[ nt.pins[j].corr_id ].tier;
			if( max_tier < nt.mods[ nt.pins[j].corr_id ].tier )
				max_tier = nt.mods[ nt.pins[j].corr_id ].tier;
		}

		if( (max_tier-min_tier) <= 0 ) continue;

		for( int j = 0; j < (max_tier-min_tier); j++ )
		{
			TSV_count[j]++;
		}
	}

	int max_area = 0;

	for( int i = 0; i < metis_nparts; i++ )
	{
		if( i < metis_nparts - 1 ) max_area += TSV_count[i] * TSV_AREA;

		max_area += mod_area[i];
	}

	max_area = (int)( max_area * (1.0 + max_WS) / metis_nparts );

	for( int i = 0; i < metis_nparts; i++ )
	{
		if( i < metis_nparts - 1 )
		{
			if( max_area < mod_area[i] + TSV_count[i] * TSV_AREA )
				return false;
		}
		else
		{
			if( max_area < mod_area[i] )
				return false;
		}
	}

	delete [] mod_area;
	delete [] TSV_count;

	return true;
}

void assign_TIER( NETLIST &nt )
{
	int num_vertex = nt.num_mod + nt.num_pad;
	int num_edge = 0;
	int num_cut = 0;
	int options[9] =
		{
			1,				// 0: default values, 1: specified values
			metis_Nruns,	// Nruns
			1,				// CType
			1,				// RType
			2,				// VType
			0,				// Reconst
			1,				// Pre-assignment
			metis_Rseed,	// random seed
			0				// debug info
		};

//	cout<<	nt.num_mod <<"  "<< nt.num_pad<<"  "<<nt.num_net<<"  "<<nt.num_pin<<endl;

	int *vwgts = new int[nt.num_mod + nt.num_pad]();
	int *part  = new int[nt.num_mod + nt.num_pad]();
	int *eptr  = new int[nt.num_net + 1]();
	int *eind  = new int[nt.num_pin]();

	for( int i = 0; i < nt.num_mod; i++ )
	{
		if(nt.mods[i].area> nt.totalModArea/nt.Layer/4)
			nt.mods[i].area/=nt.Layer;
		vwgts[i] = nt.mods[i].area;

		part[i] = MOVABLE;
	}


	for( int i = nt.num_mod; i < nt.num_mod + nt.num_pad; i++ )
	{
		vwgts[i] = 0;

		part[i] = FIXED;
	}

	int num_node = 0;
	for( int i = 0; i < nt.num_net; i++ )
	{
		int temp_num = num_node;
		int num_movable = 0;

		for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
		{
			if( !nt.pins[j].is_pad )
			{
//                cout<<" corr_id="<<nt.pins[j].corr_id<<endl;
//                cout<<"j="<<j<<"   id="<<nt.pins[j].id<<endl;
//                getchar();
				eind[num_node] = nt.pins[j].corr_id;
				num_movable++;
			}
			else
			{
				eind[num_node] = nt.pins[j].corr_id + nt.num_mod;
			}
			num_node++;
		}

		if( num_movable != 0 )
		{
			num_edge++;
			eptr[num_edge] = num_node;
		}
		else
		{
			num_node = temp_num;
		}
	}

	HMETIS_PartRecursive(num_vertex, num_edge, vwgts, eptr, eind, NULL, metis_nparts, metis_ubfactor, options, part, &num_cut);

	for( int i = 0; i < nt.num_mod; i++ )
	{
		nt.mods[i].tier = part[i];
	}
	delete [] vwgts;
	delete [] part;
	delete [] eptr;
	delete [] eind;
}

void dump_TIER( string File, NETLIST &nt, bool success, char *addName, bool has_addName , class Solution *ptr, int nFile)
{

	char *bench = new char[File.length() + 1];

	strcpy(bench, File.c_str());

	char file[50] = "\0";
    char cfile[4] = "" ;
    stringstream ss;
	strcpy( file, bench );

	if( has_addName )
	{
		strcat( file, "_" );
		strcat( file, addName );
	}
    if(nFile >= 0 )
    {
        ss<<nFile;
        ss>>cfile;
		strcat( file, "_" );
        strcat(file, cfile);
    }

	strcat( file, ".layer" );

	cout << " Dump : "<< file <<endl;
	ofstream fout;
	fout.open( file );
	if( !fout.is_open() )
	{
		cout << "error  : unable to open " << file << endl;
		exit(EXIT_FAILURE);
	}

	if( success )
	{
		fout << "NumLayer : " << metis_nparts << endl;
		for( int i = 0; i < nt.num_mod; i++ )
		{
//			fout << nt.mods[i].name << "\t" << nt.mods[i].tier << endl;
            fout << ptr->tmp_node[i].name << "\t" << ptr->tmp_node[i].tier << endl;
		}
	}
	else
	{
		fout << "fail" << endl;
	}

	fout.close();
	delete [] bench;
}

void dump_Block( char *bench, NETLIST &nt, bool success, char *addName, bool has_addName)
{
	char file[50] = "\0";
	char InFile[50] = "\0";
    stringstream ss;
    string s,temp;
    int w=0,h=0;
	strcpy( file, bench );
    strcpy( InFile, bench );

	if( has_addName )
	{
		strcat( file, "_" );
		strcat( file, addName );
	}
    strcat( file, "_stack.blocks" );
	strcat( InFile, ".blocks" );

	ofstream fout;
	fout.open( file );
    ifstream fin;
	fin.open( InFile );
	if( !fout.is_open() )
	{
		cout << "error  : unable to open " << file << endl;
		exit(EXIT_FAILURE);
	}
    if(!fin)
	{
		cout << "error  : unable to open " << InFile << endl;
		exit(EXIT_FAILURE);
	}
	if( success )
	{
        for(int j=0; j<9 ; j++){
            getline(fin, s);
            fout<<s<<endl;
        }
		for( int i = 0; i < nt.num_mod; i++ )
		{
            if(nt.mods[i].nLayer == 1 )
            {
                getline(fin, s);
                fout<<s<<endl;
            }
            else
            {
                fin>>temp>>temp>>temp;
                fin>>temp>>temp;            /// drop (0, 0)
                fin>>temp>>temp;            /// drop (0, H)
                fin >> s;		              /// read (W,
				w = atoi( s.substr(1, s.size()-2).c_str() );
                fin >> s;	         		  /// read H)
				h = atoi( s.substr(0, s.size()-1).c_str() );
				getline(fin, temp);           /// drop (W, 0) & something other
                fout<<"sb"<<i<<" hardrectilinearstack 4 (0, 0) "<<"(0, "<<h<<") ("<<w<<", "<<h<<") ("<<w<<", 0) "<<nt.mods[i].nLayer<<endl;
            }
//			fout << nt.mods[i].name << "\t" << nt.mods[i].tier << endl;
		}
		while(getline(fin, s))    ///for block file terminal
		{
            fout<<s<<endl;
		}
	}
	else
	{
		fout << "fail" << endl;
	}

	fout.close();
}

void CalculateArea( NETLIST &nt )
{
	int *mod_area = new int[ metis_nparts ]();
	for( int i = 0; i < nt.num_mod; i++ )
	{
		mod_area[ nt.mods[i].tier ] += nt.mods[i].area;
	}
	int *TSV_count = new int [ metis_nparts-1 ]();
	for( int i = 0; i < nt.num_net; i++ )
	{
		int max_tier = 0;
		int min_tier = MAX_INT;

		for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
		{
			if( nt.pins[j].is_pad )
			{
				min_tier = 0;
				continue;
			}

			if( min_tier > nt.mods[ nt.pins[j].corr_id ].tier )
				min_tier = nt.mods[ nt.pins[j].corr_id ].tier;
			if( max_tier < nt.mods[ nt.pins[j].corr_id ].tier )
				max_tier = nt.mods[ nt.pins[j].corr_id ].tier;
		}

		if( (max_tier-min_tier) <= 0 ) continue;

		for( int j = 0; j < (max_tier-min_tier); j++ )
		{
			TSV_count[j]++;
		}
	}

	int max_area = 0;

	for( int i = 0; i < metis_nparts; i++ )
	{
		if( i < metis_nparts - 1 ) max_area += TSV_count[i] * TSV_AREA;

		max_area += mod_area[i];
	}

	max_area = (int)( max_area / (1.0 - max_WS) / metis_nparts );

	ofstream fout( "data", ios::app );

	float *ws = new float[metis_nparts]();
	float temp = 3e+20;
	for( int i = 0; i < metis_nparts; i++ )
	{
		if( i < metis_nparts - 1 )
		{
			ws[i] = (max_area - (mod_area[i] + TSV_count[i] * TSV_AREA))/(float)max_area;
		}
		else
		{
			ws[i] = (max_area - mod_area[i] )/(float)max_area;
		}

		if( temp > ws[i] )
		{
			temp = ws[i];
		}
	}

	fout << "rseed: " << metis_Rseed << " " << "area: " << max_area << " " << "minWS: " << temp << endl;

	fout.close();

	delete [] ws;
	delete [] mod_area;
	delete [] TSV_count;
}

void get_area_plusTSV( NETLIST &nt , int *modarea , int *TSV_count)
{

    TSVCount( nt , TSV_count);
    for( int i = 0; i < metis_nparts; i++ )
	{
        modarea[i]=0;
	}
    for(int i=0;i<nt.num_mod;i++){
        for( int j=0; j < nt.mods[i].nLayer ; j++){
            modarea[nt.mods[i].tier + j ] += nt.mods[i].area;
        }
//     modarea[nt.mods[i].tier]+=nt.mods[i].area;
    }
    for( int i = 0; i < metis_nparts-1; i++ )
	{
            modarea[i] += TSV_count[i] * TSV_AREA;
	}
}

void get_area_plusTSV_sol( NETLIST &nt , Solution &sol ,int *modarea , int *TSV_count)
{
    TSVCount_sol( nt , sol ,TSV_count);
    for( int i = 0; i < metis_nparts; i++ )
	{
        modarea[i]=0;
	}
    for(int i=0;i<nt.num_mod;i++){
        for( int j=0; j < nt.mods[i].nLayer ; j++){
            modarea[sol.tmp_node[i].tier + j ] += sol.tmp_node[i].area;
        }
//        modarea[sol.tmp_node[i].tier]+=sol.tmp_node[i].area;
    }
    for( int i = 0; i < metis_nparts-1; i++ )
	{
            modarea[i] += TSV_count[i] * TSV_AREA;
	}
}

int CalculateTSV( NETLIST &nt , int *TSV_count){
    int nTSV=0;
    for(int i=0; i < nt.Layer-1 ;i++)
        nTSV+=TSV_count[i];
    return nTSV;
}

int CalculateTSV_sol(NETLIST &nt ,Solution &sol){
     int nTSV=0;
    int maxL=0,minL=0;
    for( int i = 0; i < nt.num_net; i++ ){
        maxL=0;
        minL=99;
        for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
		{
			if( !nt.pins[j].is_pad )
			{
				int modID = nt.mod_NameToID[nt.pins[j].name];
                    if( sol.tmp_node[modID].tier + sol.tmp_node[modID].nLayer - 1 > maxL )
                    {
                        maxL = sol.tmp_node[modID].tier + sol.tmp_node[modID].nLayer - 1;
                    }

				if( sol.tmp_node[modID].tier <= minL )
				{
					minL = sol.tmp_node[modID].tier;
				}
			}
			else
			{
				int padID = nt.pad_NameToID[nt.pins[j].name];
				minL = nt.pads[padID].tier;
			}
            if( (maxL - minL) > 0 )
            {
                nTSV += maxL - minL;
//                cout<<"maxL= "<<maxL<<" ,minL= "<<minL<<" ,maxL - minL= "<<maxL - minL<<" ,nTSV+="<<nTSV<<endl;
			}

        }
    }
    return nTSV;
}


bool modarea_smalltobig(MODULE a, MODULE b)
{
        return a.area < b.area;
}


///f///create a benchmark for stacked module
int TestBench_StackedModule(NETLIST &nt)
{
    double start_time=double(clock())/CLOCKS_PER_SEC;
    printf("\033[1;46m/========== TestBench_StackedModule %.2f ==========\\\033[0m\n",start_time);


    int stack_num=(nt.num_mod)/(nt.Layer*30)-1;
    int stack_amplify=0;
    int hard_num=0; //nt.nSoft/3 ;
    int hard_amplify=0;
    int folding_num=(nt.nHard+nt.nSoft-nt.nTSV)/(nt.Layer*10);  //0
    int folding_amplify=0;


    cout<<"* stacked module: num="<<stack_num<<", amplify*="<<(1+stack_amplify)*100<<"%\n";
    cout<<"* hard module: num="<<hard_num<<", amplify*="<<(1+hard_amplify)*100<<"%\n";
    cout<<"* folding module: num="<<folding_num<<", amplify*="<<(1+folding_amplify)*100<<"%\n";
    map < int,int > area2modID;
    vector<MODULE> tempmod=nt.mods;
    sort(tempmod.begin(), tempmod.end(), modarea_smalltobig);
    for(int i = 0; i < (int)tempmod.size(); i++)
    {
        int area=tempmod[i].area*1000;
        while(area2modID.find( area ) != area2modID.end())
        {
            area++;
            //cout<<i<<" "<<area<<endl;
            //getchar();
        }
        area2modID[area]=tempmod[i].id;

    }


 /*   for(int i = 0; i < (int)nt.mods.size(); i++)
    {
        int area=nt.mods[i].area*1000;
        while(area2modID.find( area ) != area2modID.end())
        {
            area++;
            //cout<<i<<" "<<area<<endl;
            //getchar();
        }
        area2modID[area]=i;

    }*/

    int count_num(0),tmpArea;

    tmpArea =  tempmod[ tempmod.size()-1 ].area;
//    cout<<" area="<<tmpArea<<endl;
//    getchar();
//    nt.mods[ tempmod[ tempmod.size()-1 ].id ].area/=2;
//    nt.totalModArea+=nt.mods[ tempmod[ tempmod.size()-1 ].idMod ].modArea-tmpArea;
//    nt.mods[ tempmod[ tempmod.size()-1 ].idMod ].modW = nt.mods[ tempmod[ tempmod.size()-1 ].idMod ].modH = (int)(sqrt(nt.mods[ tempmod[ tempmod.size()-1 ].idMod ].modArea) * AMP_PARA)+1;
//    nt.mods[ tempmod[ tempmod.size()-1 ].idMod ].modL=0;
    nt.mods[ tempmod[ tempmod.size()-1 ].id ].nLayer=nt.Layer;
    stack_num--;
    for(map<int, int>::reverse_iterator rit = area2modID.rbegin() ; rit != area2modID.rend() ; rit++)
    {
        if(stack_num>0)
        {
            tmpArea=nt.mods[ rit->second ].area;
		nt.mods[ rit->second ].nLayer = 2;
//            nt.mods[ rit->second ].area+=nt.mods[ rit->second ].area*stack_amplify;
		nt.mods[ rit->second ].area/=nt.mods[ rit->second ].nLayer;    ///for ibm
//            nt.totalModArea+=nt.mods[ rit->second ].area-tmpArea;
//            nt.mods[ rit->second ].modW = nt.mods[ rit->second ].modH = (int)(sqrt(nt.mods[ rit->second ].area) * AMP_PARA)+1;
//            nt.mods[ rit->second ].modL=0;
            ///y///12.2
 //           nt.mods[ rit->second ].nLayer=nt.nLayer;

            if( nt.mods[ rit->second ].nLayer +  nt.mods[ rit->second ].tier  > nt.Layer)
            {
                if( nt.Layer - nt.mods[ rit->second ].nLayer >=0 )
                    nt.mods[ rit->second ].tier = nt.Layer - nt.mods[ rit->second ].nLayer;
                else
                {
                    cout<<"error : Stack > chip.Layer" << endl;
                    exit(EXIT_FAILURE);
                }
            }
            stack_num--;
        }
        else if( folding_num>0   )
        {
            tmpArea=nt.mods[ rit->second ].area;
            nt.mods[ rit->second ].area+=nt.mods[ rit->second ].area*folding_amplify;
//            nt.totalModArea+=nt.mods[ rit->second ].modArea-tmpArea;
//            nt.mods[ rit->second ].modW = nt.mods[ rit->second ].modH = (int)(sqrt(nt.mods[ rit->second ].modArea) * AMP_PARA)+1;
	     nt.mods[ rit->second ].nLayer=2;
	     if(nt.mods[ rit->second ].tier + nt.mods[ rit->second ].nLayer > nt.Layer )
	     {
		int nrand=rand()%2;
		if(nrand)
			nt.mods[ rit->second ].tier=nt.Layer - nt.mods[ rit->second ].nLayer;
		else
			nt.mods[ rit->second ].tier=0;
	     }
//            if(nt.mods[ rit->second ].type=='1')
//            {
//                nt.mods[ rit->second ].type='0';
//                nt.nSoft--;
//                nt.nHard++;
//            }
            folding_num--;
        }
//        else if(hard_num>0)
//        {
//            if(nt.mods[ rit->second ].type=='1')
//            {
//
//                tmpArea=nt.mods[ rit->second ].modArea;
//                nt.mods[ rit->second ].modArea+=nt.mods[ rit->second ].modArea*hard_amplify;
//                nt.totalModArea+=nt.mods[ rit->second ].modArea-tmpArea;
//                nt.mods[ rit->second ].modW = nt.mods[ rit->second ].modH = (int)(sqrt(nt.mods[ rit->second ].modArea) * AMP_PARA)+1;
//
//                nt.mods[ rit->second ].type='0';
//                nt.nSoft--;
//                nt.nHard++;
//            }
//            hard_num--;
//        }
        else
        {
            break;
        }

        cout<<count_num<<" : "<<rit->first<<" "<<rit->second<<endl;
        count_num++;

    }

    area2modID.clear();


    double end_time=(double)(clock())/CLOCKS_PER_SEC;
    printf("\033[1;46m\\========== TestBench_StackedModule %.2f ==========/\033[0m\n",end_time);
    return 0;
}

static bool cmpByPd(pair<int,double> a, pair <int,double> b){
			return a.second > b.second;
}

void findHotModule(NETLIST &nt, double hot_rate)
{
    vector < pair <int,double> > power_vector ;
    //double hot_rate = 0.05 ; ///<Top x% pd set to hot module;

	power_vector.resize(nt.num_mod);
	for(int i = 0 ; i <nt.num_mod ; i++)
	{
		pair <int, double> temp ;
		temp.first = i ;
		temp.second = nt.mods[i].pd;
		power_vector[i] = temp;
	}
	sort(power_vector.begin(),power_vector.end(),cmpByPd);

    for(int i = 0 ; i <(int)(nt.num_mod*hot_rate) ; i++)
        nt.mods[power_vector[i].first].hot_flag = true;

    nt.nHot = (int)(nt.num_mod*hot_rate);
}

void calculatePdStdDevi(NETLIST &nt)
{
    double sum=0;
    double stddevi = 0;
    double mean = 0;

    for(int i=0; i<nt.num_mod; i++)
        sum+=nt.mods[i].pd;

    mean = sum/nt.num_mod;

    for(int i=0; i<nt.num_mod; i++)
        stddevi+=pow(nt.mods[i].pd-mean, 2);

    stddevi/=nt.num_mod;
    stddevi=sqrt(stddevi);

    nt.pd_mean = mean;
    nt.pd_stddevi = stddevi;

    //cout << stddevi << endl;
}
