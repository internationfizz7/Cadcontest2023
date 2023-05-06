/*!

 * \file	structure.cpp
 * \brief	data structure Parser
 *
 * \author	NCKU SEDA LAB PYChiu
 * \version	3.1
 * \date	2016.02.05
 */
#include <iostream>
#include <fstream>
#include <sstream>

//#include "structure.cpp"

#include "dataProc.h"

//#define AMP_PARA 1000			///< constant to enlarge data (avoid rounding problem)
//#define HARD_BLK '0'			///< hard module
//#define SOFT_BLK '1'			///< soft module

using namespace std;

void ReadBlockFile_HB(char *blockFile, NETLIST &nt);
void ReadNetFile_HB(char *netFile, NETLIST &nt);
void ReadPlFile_HB(char *plFile, NETLIST &nt);
void ReadPtFile_HB(char *ptFile, NETLIST &nt);

void read_GSRC_BLK( char *bench, NETLIST &nt );
void read_GSRC_NET( char *bench, NETLIST &nt );
void ReadCadcontestfile( char *bench, NETLIST &nt ,Lib &lib);
//void ReadCadcontestfile( char *bench, NETLIST &nt ,Lib &lib);
///JHWu/// For HB/HB+
void ReadBlockFile_HB(char *blockFile, NETLIST &nt)
{
    char fileIn[50];
    strcpy(fileIn, blockFile);
    strcat(fileIn, ".blocks");
    //cout << fileIn << endl;

    ifstream fileRead(fileIn);

    /*if(fileRead == NULL)
    {
        cout << "[Error] cannot open " << fileIn << endl;
        exit(1);
    }*/



    char ReadBuffer[128];
    char *token = NULL;

    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);

    fileRead.getline(ReadBuffer, 127);
    token = strtok(ReadBuffer, " :\n\t");
    if(!strncmp(token, "NumSoftRectangularBlocks", 24))
    {
        token = strtok(NULL, " :\n\t");
        nt.nSoft = atoi(token);
    }

    fileRead.getline(ReadBuffer, 127);
    token = strtok(ReadBuffer, " :\n\t");
    if(!strncmp(token, "NumHardRectilinearBlocks", 24))
    {
        token = strtok(NULL, " :\n\t");
        nt.nHard = atoi(token);
    }
    nt.num_mod = nt.nSoft + nt.nHard;

    fileRead.getline(ReadBuffer, 127);
    token = strtok(ReadBuffer, " :\n\t");
    if(!strncmp(token, "NumTerminals", 12))
    {
        token = strtok(NULL, " :\n\t");
        nt.num_pad = atoi(token);
    }
    fileRead.getline(ReadBuffer, 127);
    nt.mods.resize(nt.num_mod);
    //cout << "nPad : " << nt.nPad << endl;
    nt.pads.resize(nt.num_pad);

    for(int i = 0; i<nt.nSoft; i++)
    {
        fileRead.getline(ReadBuffer, 127);
        token = strtok(ReadBuffer, " :\n\t"); // BLOCK_*
		//strcpy(nt.mods[i].moduleName,token);          ///< 2103-11-20 remove
		//strcpy(nt.mods[i].instanceName,token);
        strcpy(nt.mods[i].name,token);///f///0602
        nt.mods[i].id = i;

        nt.mod_NameToID[ nt.mods[i].name ] = i;    ///y/2016.2.15

        token = strtok(NULL, " :\n\t");       // softrectangular
        nt.mods[i].type = SOFT_BLK;
        token = strtok(NULL, " :\n\t");
        nt.mods[i].area = atoi(token);
        if(nt.mods[i].area>maxarea)
            maxarea=nt.mods[i].area;
        token = strtok(NULL, " :\n\t");
//        nt.mods[i].minAR = atoi(token);
        token = strtok(NULL, " :\n\t");
//        nt.mods[i].maxAR = atoi(token);
        nt.totalModArea += nt.mods[i].area;

        ///f///nt.flagPureSoft = 1;
        ///f///0602
//        nt.mods[i].nLayer = 1;
//        nt.mods[i].modW=(int)sqrt(nt.mods[i].modArea)*AMP_PARA;
//        nt.mods[i].modH=(int)sqrt(nt.mods[i].modArea)*AMP_PARA;
    }
    int modW=0,modH=0;
    for(int i=nt.nSoft; i<nt.nHard + nt.nSoft; i++)
    {
        fileRead.getline(ReadBuffer, 127);
        token = strtok(ReadBuffer, " :\n\t"); // BLOCK_*
		//strcpy(nt.mods[i].moduleName,token);      ///<2013-11-20 remove
		//strcpy(nt.mods[i].instanceName,token);
        strcpy(nt.mods[i].name,token);///f///0602
        nt.mods[i].id = i;

        nt.mod_NameToID[ nt.mods[i].name ] = i;    ///y/2016.2.15

        token = strtok(NULL, " :\n\t");       // hardrectilinear
        nt.mods[i].type = HARD_BLK;
        token = strtok(NULL, " :\n\t,()");    // 4
        token = strtok(NULL, " :\n\t,()");
        token = strtok(NULL, " :\n\t,()");
        token = strtok(NULL, " :\n\t,()");
        token = strtok(NULL, " :\n\t,()");
        token = strtok(NULL, " :\n\t,()");
//        nt.mods[i].modW = atoi(token);
        modW = atoi(token);
        token = strtok(NULL, " :\n\t,()");
//        nt.mods[i].modH = atoi(token);
        modH = atoi(token);
        nt.mods[i].area = modW * modH;
        if(nt.mods[i].area>maxarea)
            maxarea=nt.mods[i].area;
//        nt.mods[i].minAR = nt.mods[i].maxAR = nt.mods[i].modW / nt.mods[i].modH;
        nt.totalModArea += nt.mods[i].area;
        ///f///nt.flagPureSoft = 0;
        ///f///0602
//        nt.mods[i].nLayer = 1;
//        nt.mods[i].modW*=AMP_PARA;
//        nt.mods[i].modH*=AMP_PARA;
    }

//    string strtemp;
//    stringstream sstemp;
//    int count_pad=0;
//    while( getline(fileRead, strtemp) )
//	{
//        sstemp << strtemp;
//
//		sstemp >> strtemp;
//
//        string name = strtemp;
//		if( strtemp == "terminal" )
//        {
//				strcpy( nt.pads[count_pad].name, name.c_str() );
//
//				nt.pads[count_pad].id = count_pad;
//				nt.pad_NameToID[ nt.pads[count_pad].name ] = count_pad;
//
//				count_pad++;
//        }
//
//		sstemp.str( string() );
//		sstemp.clear();
//    }

    fileRead.close();
    return;
}

void ReadNetFile_HB(char *netFile, NETLIST &nt)
{
    char fileIn[50];
    strcpy(fileIn, netFile);
    strcat(fileIn, ".nets");
    // cout << fileIn << endl;

    ifstream fileRead(fileIn);

    /*if(fileRead == NULL)
    {
        cout << "[Error] cannot open " << fileIn << endl;
        exit(1);
    }*/
    char ReadBuffer[128];
    char *token = NULL;

    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);

    fileRead.getline(ReadBuffer, 127);
    token = strtok(ReadBuffer, " :\n\t");
    if(!strncmp(token, "NumNets", 7))
    {
        token = strtok(NULL, " :\n\t");
        nt.num_net = atoi(token);
    }

    fileRead.getline(ReadBuffer, 127);
    token = strtok(ReadBuffer, " :\n\t");
    if(!strncmp(token, "NumPins", 7))
    {
        token = strtok(NULL, " :\n\t");
        nt.num_pin = atoi(token);
    }

    fileRead.getline(ReadBuffer, 127);

    nt.nets.resize(nt.num_net);
    nt.pins.resize(nt.num_pin);

    // construct relationship between nt.nets and nt.pins
    int indexP = 0;
    for(int i = 0; i<nt.num_net; i++)
    {
        nt.nets[i].id = i;
        nt.nets[i].head = indexP;
//        nt.nets[i].flag = 0;
        fileRead.getline(ReadBuffer, 127);
        token = strtok(ReadBuffer, " :\n\t"); //NetDegree
        token = strtok(NULL, " :\n\t");
        nt.nets[i].degree = atoi(token);
        for(int j = 0; j<nt.nets[i].degree; j++)
        {
            fileRead.getline(ReadBuffer, 127);
            token = strtok(ReadBuffer, " :\n\t");
            if(token[0] == 'P')  // Overlook the Nets Connecting with the boundary
            {
                char *n = token + 4;  // p
                //cout << n << endl;
                nt.pins[indexP].id = indexP;
                nt.pins[indexP].corr_id = atoi(n); // start from PAD_0
//                nt.pins[indexP].corr_id = nt.pins[indexP].id + nt.num_mod;
//                nt.pins[indexP].corr_id = nt.pins[indexP].corr_id + nt.num_mod;


                nt.pins[indexP].is_pad = true;  ///pychiu///216
 //               nt.pad_NameToID[ nt.pads[indexP].name ] = indexP;

                indexP++;
            }
            else
            {
                char *n = token + 6;  // sb
                //cout << n << endl;
                nt.pins[indexP].id = indexP;
                nt.pins[indexP].corr_id = atoi(n) - 1; // start from BLOCK_1
                nt.mods[atoi(n)-1].num_net++;

                nt.pins[indexP].is_pad = false; ///pychiu///216

                indexP++;
            }
        }
    }
    /// construct relationship between nt.nets and nt.mods
    for(int i = 0; i<nt.num_mod; i++)  // Alloc Memory for edge array in mods
    {
        nt.mods[i].array_net = (int *)malloc(sizeof(int) * nt.mods[i].num_net);
        if(nt.mods[i].num_net>maxpin)
            maxpin=nt.mods[i].num_net;
        if(nt.mods[i].array_net == NULL)
        {
            cerr << "Out of Mem on nt.mods[" << i << "].array_net" << endl;
            exit(1);
        }
        nt.mods[i].num_net = 0;  // for next step to assign nt.mods.pNet
    }

    for(int i = 0; i<nt.num_net; i++)
    {
        for(int j = nt.nets[i].head; j<nt.nets[i].head + nt.nets[i].degree; j++)  ///corr_id
        {
            if (nt.pins[j].corr_id < nt.num_mod)
            {
                nt.mods[nt.pins[j].corr_id].array_net[nt.mods[nt.pins[j].corr_id].num_net] = i;
                nt.mods[nt.pins[j].corr_id].num_net++;
            }
        }
    }

    // buffNet used in swap process
    buffNet = (unsigned short *)malloc(sizeof(unsigned short) * nt.num_net);
    if(buffNet == NULL)
    {
        cout << "Run Out of Mem on buffNet" << endl;
    }

    ///f///
    /*maxNumNetB = 0;
    for(int i=0; i<nt.nMod; i++)
    {
        maxNumNetB = (nt.mods[i].nNet > maxNumNetB) ? nt.mods[i].nNet : maxNumNetB;
    }*/




    fileRead.close();
    return;
}


void ReadPlFile_HB(char *plFile, NETLIST &nt)
{
    char fileIn[50];
    strcpy(fileIn, plFile);
    strcat(fileIn, ".pl");
    // cout << fileIn << endl;

    ifstream fileRead(fileIn);

    /*if(fileRead == NULL)
    {
        cerr << "[Error] cannot open " << fileIn << endl;
        exit(1);
    }*/
    char ReadBuffer[128];
    char *token = NULL;

    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    fileRead.getline(ReadBuffer, 127);
    for(int i = 0; i<nt.num_mod; i++)
    {
        fileRead.getline(ReadBuffer, 127);
        token = strtok(ReadBuffer, " :\n\t");
        token = strtok(NULL, " :\n\t");
//        nt.mods[i].LeftX = atoi(token) * AMP_PARA;
        token = strtok(NULL, " :\n\t");
//        nt.mods[i].LeftY = atoi(token) * AMP_PARA;
    }

    fileRead.getline(ReadBuffer, 127);
    for(int i = 0; i<nt.num_pad; i++)
    {
	// cout << "i : " << i << endl;
        fileRead.getline(ReadBuffer, 127);
        token = strtok(ReadBuffer, " :\n\t");
		//strcpy(nt.pads[i].padName,token);       ///< 2013-11-20 remove
        strcpy(nt.pads[i].name,token);///f///0602
        nt.pads[i].id = i + nt.num_mod;
        token = strtok(NULL, " :\n\t");
//        nt.pads[i].x = (int)(atof(token) * ((float)AMP_PARA));
        token = strtok(NULL, " :\n\t");
//        nt.pads[i].y = (int)(atof(token) * ((float)AMP_PARA));
		///f///nt.pads[i].padW = 10000;
		///f///nt.pads[i].padH = 10000;
        ///f///nt.pads[i].padX = nt.pads[i].x;// / AMP_PARA;
        ///f///nt.pads[i].padY = nt.pads[i].y;// / AMP_PARA;
        ///f///nt.pads[i].orientation = 0;
    }
    fileRead.close();
    return;
}//*/
void ReadPtFile_HB(char* ptFile, NETLIST& nt)
{
    char fileIn[50];
    string temp;
    int block_number;
    map<string,double> power_map;
    vector<string> name_vector;

    strcpy(fileIn, ptFile);
    strcat(fileIn, ".pt");

    ifstream fileRead(fileIn);

    /*if(fileRead == NULL)
    {
        cerr << "[Error] cannot open " << fileIn << endl;
        exit(1);
    }*/

    while(fileRead>>temp)
    {
        if(temp.find("block") != string::npos)
        {
            fileRead >> temp >> block_number;
            break;
        }
    }

    if(block_number != nt.num_mod)
    {
        cerr << "[Error] inconsistent number of module" << endl;
        exit(1);
    }

    name_vector.reserve(nt.num_mod);

    while(fileRead>>temp)
    {
        if(temp.find("block") != string::npos)
        {
            fileRead >> temp; ///<read ":"

            for(int i=0; i<nt.num_mod; i++)
            {
                fileRead >> temp;
                power_map[temp] = 0.0;
                name_vector.push_back(temp);
            }
            break;
        }
    }

    while(fileRead>>temp)
    {
        if(temp.find("power") != string::npos)
        {
            fileRead >> temp; ///<read ":"
            double double_temp;
            for(int i=0; i<nt.num_mod; i++)
            {
                fileRead >> double_temp;
                power_map[name_vector[i]] = double_temp;
                /*cout << double_temp << endl;
                cout << name_vector[i] << " " << power_map[name_vector[i]] << endl;
                getchar();*/
            }
            break;
        }
    }

    for(int i=0; i<nt.num_mod; i++)
    {
        string str_name(nt.mods[i].name);
        nt.mods[i].pd = power_map[str_name];
    }
}




void read_GSRC_BLK( char *bench, NETLIST &nt )
{
	char file[50] = "\0";
	strcpy( file, bench );
	strcat( file, ".blocks" );

	ifstream fin;
	fin.open( file );
	if( !fin.is_open() )
	{
		cout << "error  : unable to open " << file << endl;
		exit(EXIT_FAILURE);
	}

	int count_mod = 0;
	int count_pad = 0;

	stringstream sstemp;
	string strtemp;

	while( getline(fin, strtemp) )
	{
		sstemp << strtemp;

		sstemp >> strtemp;

		if( strtemp == "" || strtemp.substr(0, 1) == "#" )
		{
			sstemp.str( string() );
			sstemp.clear();
			continue;
		}

		if( strtemp == "NumSoftRectangularBlocks" )
		{
			int num_soft = 0;
			sstemp >> strtemp >> num_soft;

			nt.num_mod += num_soft;
			nt.mods.resize( nt.num_mod );
		}
		else if( strtemp == "NumHardRectilinearBlocks" )
		{
			int num_hard = 0;
			sstemp >> strtemp >> num_hard;

			nt.num_mod += num_hard;
			nt.mods.resize( nt.num_mod );
		}
		else if( strtemp == "NumTerminals" )
		{
			sstemp >> strtemp >> nt.num_pad;

			nt.pads.resize( nt.num_pad );
		}
		else
		{
			string name = strtemp;

			sstemp >> strtemp;

			if( strtemp == "softrectangular" )
			{
				strcpy( nt.mods[count_mod].name, name.c_str() );

				sstemp >> nt.mods[count_mod].area;

				if(nt.mods[count_mod].area>maxarea)
                    maxarea=nt.mods[count_mod].area;

				nt.mods[count_mod].id = count_mod;
				nt.mod_NameToID[ nt.mods[count_mod].name ] = count_mod;

				count_mod++;
			}
			else if( strtemp == "hardrectilinear" )
			{
				int mod_w = 0;
				int mod_h = 0;

				strcpy( nt.mods[count_mod].name, name.c_str() );

				sstemp >> strtemp;				// drop 4
				sstemp >> strtemp >> strtemp;	// drop (0, 0)
				sstemp >> strtemp >> strtemp;	// drop (0, H)
				sstemp >> strtemp;				// read (W,

				mod_w = atoi( strtemp.substr(1, strtemp.size()-2).c_str() );

				sstemp >> strtemp;				// read H)

				mod_h = atoi( strtemp.substr(0, strtemp.size()-1).c_str() );

				nt.mods[count_mod].area = mod_w * mod_h;
				if(nt.mods[count_mod].area>maxarea)
                    maxarea=nt.mods[count_mod].area;
				nt.mods[count_mod].id = count_mod;
				nt.mod_NameToID[ nt.mods[count_mod].name ] = count_mod;

				count_mod++;
			}
			else if( strtemp == "terminal" )
			{
				strcpy( nt.pads[count_pad].name, name.c_str() );

				nt.pads[count_pad].id = count_pad;
				nt.pad_NameToID[ nt.pads[count_pad].name ] = count_pad;

				count_pad++;
			}
			//else
			//{
			//	cout << "warning: unrecognized content in .blocks file. " << sstemp.str() << endl;
			//}
		}

		sstemp.str( string() );
		sstemp.clear();
	}
}

void read_GSRC_NET( char *bench, NETLIST &nt )
{
	char file[50] = "\0";
	strcpy( file, bench );
	strcat( file, ".nets" );

	ifstream fin;
	fin.open( file );
	if( !fin.is_open() )
	{
		cout << "error  : unable to open " << file << endl;
		exit(EXIT_FAILURE);
	}

	stringstream sstemp;
	string strtemp;

	int count_net = 0;
	int count_pin = 0;

	int degree = 0;

	while( getline(fin, strtemp) )
	{
		sstemp << strtemp;

		sstemp >> strtemp;

		if( strtemp == "" || strtemp.substr(0, 1) == "#" )
		{
			sstemp.str( string() );
			sstemp.clear();
			continue;
		}

		if( strtemp == "NumNets" )
		{
			sstemp >> strtemp >> nt.num_net;

			nt.nets.resize( nt.num_net );
		}
		else if( strtemp == "NumPins" )
		{
			sstemp >> strtemp >> nt.num_pin;

			nt.pins.resize( nt.num_pin );
		}
		else if( strtemp == "NetDegree" )
		{
			sstemp >> strtemp >> degree;

			nt.nets[count_net].id = count_net;
			nt.nets[count_net].head = count_pin;
			nt.nets[count_net].degree = degree;

			count_net++;
		}
		else if( degree )
		{
			strcpy( nt.pins[count_pin].name, strtemp.c_str() );

			nt.pins[count_pin].id = count_pin;

			if( nt.mod_NameToID.find(strtemp) != nt.mod_NameToID.end() )
			{
				nt.pins[count_pin].corr_id = nt.mod_NameToID[strtemp];

				nt.mods[ nt.pins[count_pin].corr_id ].num_net++;
			}
			else if( nt.pad_NameToID.find(strtemp) != nt.pad_NameToID.end() )
			{
				nt.pins[count_pin].corr_id = nt.pad_NameToID[strtemp];

				nt.pins[count_pin].is_pad = true;
			}
			else
			{
				cout << "error  : unable to find component " << "\"" << strtemp << "\"" << endl;
				exit(EXIT_FAILURE);
			}

			degree--;
			count_pin++;
		}
        /// allocate memory for nets which link to mod[index] (mod[index].array_net)
		for(int i=0;i<nt.num_mod;i++)
		{
            try
            {
                nt.mods[i].array_net = new int [nt.mods[i].num_net];
                if(nt.mods[i].num_net>maxpin)
                    maxpin=nt.mods[i].num_net;
            }
            catch( bad_alloc &bad )
            {
                cout << "error  : run out of memory on tmpnt.mods[" << nt.mods[i].id << "].array_net" << endl;
                exit(EXIT_FAILURE);
            }
            nt.mods[i].num_net=0;
		}
        /// construct the relationship between nets and modules
        for( int i = 0; i < nt.num_net; i++ )
        {
            for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
            {
                if( nt.mod_NameToID.find( nt.pins[j].name ) != nt.mod_NameToID.end() )
                {
                    nt.mods[ nt.pins[j].corr_id ].array_net[ nt.mods[nt.pins[j].corr_id].num_net ] = nt.nets[i].id;
                    nt.mods[ nt.pins[j].corr_id ].num_net++;
                }
            }
        }


		//else
		//{
		//	cout << "warning: unrecognized content in .blocks file. " << sstemp.str() << endl;
		//}

		sstemp.str( string() );
		sstemp.clear();
	}
    /*for(int i=0;i<20;i++){

        cout<<nt.mods[i].name<<" "<<nt.mods[i].mod_h<<" "<<nt.mods[i].mod_w<<" "<<nt.mods[i].id<<" "<<nt.mod_NameToID[nt.mods[i].name]<<"\n";        
        for(int j=0;j<nt.mods[i].num_net;j++){
        cout<<"    "<<nt.mods[i].array_net[j];
        }
        cout<<"\n";
     } */
	fin.close();
}
void ReadCadcontestfile( char *bench, NETLIST &nt , Lib &lib)
{
    //cout<<"haha";
	char file[50] = "\0";
	strcpy( file, bench );
	strcat( file, ".cad" );

	ifstream fin;
	fin.open( file );
	if( !fin.is_open() )
	{
		cout << "error  : unable to open " << file << endl;
		exit(EXIT_FAILURE);
	}

	int count_mod = 0;
	int count_net = 0;
    bool read1=false; //之後註解掉
	stringstream sstemp;
	string strtemp;

	while( getline(fin, strtemp) )
	{
		sstemp << strtemp;

		sstemp >> strtemp;

		if( strtemp == "" || strtemp.substr(0, 1) == "#" )
		{
			sstemp.str( string() );
			sstemp.clear();
			continue;
		}
        else if(strtemp=="NumTechnologies"){
            
            int num_tech=0;
            sstemp>>num_tech;
            //nt.tech+=num_tech;
            sstemp.str( string() );
		    sstemp.clear();
            
        }
        //先假設只讀一個tech 之後要改
        //只讀了大小 其他資訊陸續補上
        else if(strtemp=="Tech"){
             
            if(read1==false){
            int library=0;
            sstemp>>strtemp>>library;
            
            lib.lib_cell.resize(library);
            sstemp.str( string() );
			sstemp.clear();
            
            for(int i=0;i<library;i++){
                getline(fin, strtemp);
                sstemp << strtemp;

		        sstemp >> strtemp;
                int p=0;//pin_num
                string name;
                int w=0;
                int h=0;
                if(strtemp=="LibCell"){
                   
                    sstemp/*>>strtemp*/>>name>>w>>h>>p;
                    lib.lib_cell[i].id=i;
                    lib.lib_cell[i].h=h;
                    lib.lib_cell[i].w=w;
                    lib.lib_cell[i].p=p;
                   
                    
                    strcpy( lib.lib_cell[i].name, name.c_str() );
                    lib.lib_NameToID[lib.lib_cell[i].name]=i;
                    sstemp.str( string() );
			        sstemp.clear();
                    for(int j=0;j<p;j++){
                        getline(fin, strtemp);
                        sstemp.str( string() );
                        sstemp.clear();
                    }

                    //cout<<lib.lib_cell[library].w<<" "<<lib.lib_cell[library].h<<" "<<lib.lib_cell[library].p<<"haha\n";
                }
                else{
                    cout<<"error! Should be LibCell";
                }
                sstemp.str( string() );
                sstemp.clear();
                //cout<<lib.lib_cell[library].w<<" ";
            }


            read1=true;
            }
            sstemp.str( string() );
		    sstemp.clear();
        }
		else if( strtemp == "NumInstances" )
		{
            
			int num_instances = 0;
			sstemp >> num_instances;
			nt.num_mod += num_instances;
            //cout<<"nt.num_mod "<<nt.num_mod<<"\n";
			nt.mods.resize( nt.num_mod );
            sstemp.str( string() );
			sstemp.clear();
            for(int i=0;i<nt.num_mod;i++){
                getline(fin, strtemp);
                sstemp << strtemp;

		        sstemp >> strtemp;
                if(strtemp=="Inst"){
                    string name ;
                    string type ;
			        sstemp >> name>>type;
                    strcpy( nt.mods[count_mod].name, name.c_str() );
                    //cout<<"nt.mods["<<count_mod<<"].name "<<nt.mods[count_mod].name<<"\n";
                    if( lib.lib_NameToID.find(type) != lib.lib_NameToID.end() ){
                        nt.mods[count_mod].mod_w=lib.lib_cell[lib.lib_NameToID[type]].w;
                        nt.mods[count_mod].mod_h=lib.lib_cell[lib.lib_NameToID[type]].h;
                        //cout<<nt.mods[count_mod].mod_w<<" "<<nt.mods[count_mod].mod_h<<"\n";
                    }
                    else{
                        cout<<"error! No such Library\n";
                    }
                    nt.mods[count_mod].area=nt.mods[count_mod].mod_w*nt.mods[count_mod].mod_h;
                    nt.totalModArea += nt.mods[count_mod].area;
                    if(nt.mods[count_mod].area>maxarea)
                     maxarea=nt.mods[count_mod].area;
                    nt.mods[count_mod].id=count_mod;
                    nt.mod_NameToID[nt.mods[count_mod].name]=count_mod;
                    count_mod++;
                }
                else{
                    cout<<"error! Inst not enough for NumInstances!\n";
                }
                sstemp.str( string() );
			    sstemp.clear();
            }
            if(num_instances!=count_mod){
                cout<<"error! Inst not enough for NumInstances!\n";
            }
            /*else{
                count_mod=0;
            }*/
		}
		else if( strtemp == "NumNets" )
		{
            //int cc=0;
			int num_nets=0;
            sstemp >> num_nets;
            nt.num_net=num_nets;
            nt.nets.resize(nt.num_net);
            //int count_net = 0;
            int count_pin = 0;

            int degree = 0;
            sstemp.str( string() );
			sstemp.clear();
            for(int i=0;i<num_nets;i++)
            {
                getline(fin, strtemp);
                sstemp << strtemp;

                sstemp >> strtemp;

                if( strtemp == "" || strtemp.substr(0, 1) == "#" )
                {
                    sstemp.str( string() );
                    sstemp.clear();
                    continue;
                }

                else if( strtemp == "Net" )
                {
                    sstemp >> strtemp >> degree;
                    strcpy( nt.nets[i].name, strtemp.c_str() );
                    nt.nets[i].id = i;
                    nt.nets[i].head = count_pin;
                    nt.nets[i].degree = degree;
                    sstemp.str( string() );
                    sstemp.clear();
                    //count_net++;
                    for(int j=0;j<degree;j++){
                        getline(fin, strtemp);
                        sstemp << strtemp;

                        sstemp >> strtemp;
                        if(strtemp=="Pin"){
                            sstemp>>strtemp;
                            string f_s("/");
                            std::size_t found=strtemp.find(f_s);
                            int find_pos=static_cast<int>(found);
                            string fiind=strtemp.substr(0,find_pos);
                            if( nt.mod_NameToID.find(fiind) != nt.mod_NameToID.end() )
                            {
                                //cout<<"founded!\n";
                                nt.pins.resize(count_pin+1);
                                nt.pins[count_pin].id=count_pin;
                                strcpy( nt.pins[count_pin].name, fiind.c_str() );
                                nt.pins[count_pin].corr_id = nt.mod_NameToID[fiind];

                                nt.mods[ nt.pins[count_pin].corr_id ].num_net++;
                            }
                            else{
                                cout<<"error! cant find this mod\n";
                            }
                            count_pin++;
                        }
                        else{
                            cout<<"error! There should be enough Pin lines, not enough\n";
                        }
                        sstemp.str( string() );
			            sstemp.clear();
                    }
                }
                sstemp.str( string() );
			    sstemp.clear();
		    }
		
		

		sstemp.str( string() );
		sstemp.clear();
        nt.num_pin=nt.pins.size();
	    }
        else
		{
			sstemp.str( string() );
			sstemp.clear();
			continue;
		}
        
            
    }
    /*for(int i=0;i<nt.pins.size();i++){

        cout<<nt.mods[ nt.pins[i].corr_id ].name<<" "<<i<<"  ";
    }*/
    //cout<<nt.mods.size()<<" "<<nt.nets.size()<<" "<<nt.pins.size();
    /// allocate memory for nets which link to mod[index] (mod[index].array_net)
    for(int i=0;i<nt.num_mod;i++)
                {
                    try
                    {
                        nt.mods[i].array_net = new int [nt.mods[i].num_net];
                        if(nt.mods[i].num_net>maxpin)
                            maxpin=nt.mods[i].num_net;
                    }
                    catch( bad_alloc &bad )
                    {
                        cout << "error  : run out of memory on tmpnt.mods[" << nt.mods[i].id << "].array_net" << endl;
                        exit(EXIT_FAILURE);
                    }
                    nt.mods[i].num_net=0;
                }
                /// construct the relationship between nets and modules
    for( int i = 0; i < nt.num_net; i++ )
                {
                    for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
                    {
                        if( nt.mod_NameToID.find( nt.pins[j].name ) != nt.mod_NameToID.end() )
                        {
                            
                            nt.mods[ nt.pins[j].corr_id ].array_net[ nt.mods[nt.pins[j].corr_id].num_net ] = nt.nets[i].id;
                            nt.mods[ nt.pins[j].corr_id ].num_net++;
                        }
                        else{
                            
                        }
                    }
                }


                //else
                //{
                //	cout << "warning: unrecognized content in .blocks file. " << sstemp.str() << endl;
                //}



    //test read in 
    /*for(int i=0;i<10;i++){
        cout<<nt.nets[i].name<<" ";
    }*/
    /*for(int i=0;i<nt.mods.size();i++){

        cout<<nt.mods[i].name<<" "<<nt.mods[i].mod_h<<" "<<nt.mods[i].mod_w<<" "<<nt.mods[i].id<<" "<<nt.mod_NameToID[nt.mods[i].name]<<"\n";        
        for(int j=0;j<nt.mods[i].num_net;j++){
        //cout<<"    "<<nt.nets[nt.mods[i].array_net[j]].name;
        cout<<"    "<<nt.mods[i].array_net[j];
        }
        cout<<"\n";
        
     }   
    for(int i=0;i<nt.nets.size();i++){
        cout<<nt.nets[i].name<<" "<<nt.nets[i].id<<" "<<nt.nets[i].head<<" "<<nt.nets[i].degree<<" \n";
    }
    for(int i=0;i<nt.pins.size();i++){
        cout<<nt.pins[i].name<<" "<<nt.pins[i].id<<" "<<nt.pins[i].corr_id<<" \n";
    }*/


    
}
