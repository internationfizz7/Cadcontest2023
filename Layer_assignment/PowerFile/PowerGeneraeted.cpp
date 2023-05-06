#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <cmath>
#include <stack> //new
#include <stdlib.h> //new
#include <algorithm>

using namespace std;


class nets
{
public:
    int netdegree;
    int mod_index;
    int mod_area;

};
class POWER
{
public :
    vector <double> power;
    vector <double> Pdensity;
    string block_name;

};

//================Fcn Title=============================//
void read_block(string file_name,map <string, int > &area,bool StoreName);
void read_net(string file,map <string, int > &area);

//==========Gloabl Containter==========================//
vector <nets> mod_Nnet;
vector <string> BlockNameGSRC;
vector <string> BlockNameITRI;
map <string,int > name_to_id;

bool com( pair<double,double> a, pair<double,double> b)
{
    return a.first > b.first;
}
bool compare( nets a, nets b )
{
    if(a.mod_area == b.mod_area)
    {

        return a.netdegree>  b.netdegree;
    }
    else
        return a.mod_area< b.mod_area;

}

void read_block(string file_name,map <string, int > &area,bool StoreName)
{
    string fileName = file_name+ ".blocks";
    ifstream fin1(fileName.c_str());
    if(!fin1)
        cout<<"open error"<<endl;

    string drop;
    int soft_number ;
    int hard_number;
    while(drop!="NumSoftRectangularBlocks")
        fin1>>drop;

    fin1>> drop >> soft_number;
    fin1>> drop >> drop >> hard_number;
    fin1>> drop >> drop >> drop;


    for(int i=0; i< soft_number ; i++)
    {
        string block_name;
        int block_area ;
        fin1>> block_name >> drop >>block_area >>drop>>drop;
        //  cout<<"block name "<<block_name<<endl;

        area[block_name]=block_area;

        if(StoreName)
        {
            name_to_id[block_name]=i;
            BlockNameGSRC.push_back(block_name);
        }

        else
            BlockNameITRI.push_back(block_name);

    }
    for(int i=0; i< hard_number ; i++)
    {
        string block_name;
        int w;
        int h;
        int block_area;
        fin1>> block_name >> drop >> drop >> drop >> drop >> drop >> drop;
        fin1>> drop;
        h = atoi(drop.substr(1, drop.size()-2).c_str());
        fin1 >> drop;
        w = atoi(drop.substr(0, drop.size()-1).c_str());
        block_area = w * h;
        fin1>>drop >> drop;

        area[block_name ] = block_area;

        if(StoreName)
        {
            name_to_id[block_name]=i;
            BlockNameGSRC.push_back(block_name);
        }

        else
            BlockNameITRI.push_back(block_name);
    }
    fin1.close();


}
void read_net(string file,map <string, int > &area)
{
    ifstream fin((file+".nets").c_str());
    mod_Nnet.resize(area.size());
    string drop;
    while(drop!="NumNets")
        fin>>drop;
    int num_nets,num_pins;
    fin>>drop>>num_nets;
    fin>>drop>>drop>>num_pins;
    for(int i=0 ; i< num_nets; i++)
    {
        int net_degree;
        fin>> drop>>drop >>net_degree;
        for(int j=0; j<net_degree; j++)
        {
            string temp_name ;
            int block_area;
            fin>>temp_name>> drop;
            map<string, int>::iterator iter1;
            iter1 = area.find(temp_name);
            if(iter1!=area.end()) // mod name
            {
                block_area= iter1->second;
                int mod_index ;
                map<string, int>::iterator iter2;
                iter2= name_to_id.find(temp_name);
                mod_index= iter2->second;
                mod_Nnet[mod_index].mod_index = mod_index;
                mod_Nnet[mod_index].netdegree= mod_Nnet[mod_index].netdegree +1;
                mod_Nnet[mod_index].mod_area =block_area;

            }
            else
            {

            }



        }

    }
    cout<<"Parser Finish NET"<<endl;
//    getchar();


}
int Bin(int area_sum, double ws, int number_block)
{
    int AMP_PARA = 1000;
    int maxArea =area_sum;

    int ChipWidth  = (int)(sqrt( 1.0/(1.0 - ws) *  maxArea / 1) * AMP_PARA);
    int ChipHeight = (int)(sqrt( 1.0/(1.0 - ws) * maxArea * 1) * AMP_PARA);



    double AvgModLength = sqrt((double)area_sum /number_block);

    int numBin = (int)((double)ChipHeight /(double) AMP_PARA/AvgModLength );

    int bin_para=5;
    int  nBinRow,nBinCol;
    nBinRow = nBinCol = numBin * bin_para;

    cout<<"nBinRow  "<<nBinRow<<endl;

    int binW = (int)((ChipWidth/((float)AMP_PARA)) / ((float)nBinCol) * AMP_PARA);
    int binH = (int)((ChipHeight/((float)AMP_PARA)) / ((float)nBinRow) * AMP_PARA);
    return binW;

}
void GeneratePower(int GeneratePowNum,map<string,int> &area,map<string,int> &manual_area, int BlockCaseNum,int ampFactor, string InputPath ,int dy_time)
{


    //===============usr specified parameters==================//
    int case_num = 9 ;
    int choose = dy_time ; // you can choose you what you want number of power
    int fill_num = 1; // user-specify -> ex. n100 -> = (19+' 1->(fill_num)') +(19+1) *4
    int copy_num = (BlockCaseNum/20)-1 ; // user-specify -> ex. n100 -> = (19+1)+ (19+1)* "4" (copy_num)
    int TargetNum= BlockCaseNum; // ibm01 = 911, n200=200,n100=100
    int output_case = GeneratePowNum;
    int hot_spot_num = 3 ; //Default
    double ws = 0.2;
    //========================================================//
    vector <int > switch_index;
    switch_index.push_back(0);
    srand(time(NULL));

    hot_spot_num = GeneratePowNum / 20 ; // every 20 will occur a hotspot

    map <string,POWER > DP_generate;



    for(int i=1 ; i<= case_num ; i++)
    {
        stringstream ss;
        string label;
        ss<<i;
        ss>> label;
        string fileIn;
        string DataPath =  InputPath ;

        fileIn = DataPath+ "TrainingDataFloorplan/out"+ label +".pt";

        cout<<"reading...."<<endl;
        cout<<"case : "<<i<<endl;
        cout<<"filename "<<fileIn<<endl;

        ifstream fin(fileIn.c_str());

        if(!fin)
        {
            cerr << "Error in opening " << fileIn << endl;
            exit(-1);
        }
        string trash;
        string unit;
        int number_blocks;
        vector <POWER> power_trace_generate;

        while(!fin.eof())
        {
            fin>> trash >> trash>>unit>>trash;
            fin>> trash >> trash >>trash >>trash>>number_blocks>>trash;
            fin>> trash >> trash;
            for(unsigned int j=0; j<number_blocks; j++)
                fin>> trash; // drop the block name
            power_trace_generate.resize(GeneratePowNum);
            //=====generate power ModName====//
            for(int j=0; j< GeneratePowNum ; j++)
            {
                POWER temp;
                string block_name = BlockNameGSRC[j];

                temp.block_name=  BlockNameGSRC[j];
                power_trace_generate[j].block_name = BlockNameGSRC[j];
                if(i==1)
                    DP_generate[block_name]= temp;

            }
            //=============================//
            //========Bin Width ===========//
            fin>> trash >> trash;
            map<string, int>::iterator iter2;
            int itri_areaSum = 0;
            int gsrc_areaSum =0;

            for( iter2= area.begin(); iter2!= area.end(); iter2++)
            {
                itri_areaSum+=iter2->second;
            }
            for(iter2 =manual_area.begin() ; iter2!= manual_area.end(); iter2++)
            {

                gsrc_areaSum+= iter2->second;
            }

            int itri_binW = Bin(itri_areaSum, ws, number_blocks);
            int gsrc_binW = Bin(gsrc_areaSum, ws, GeneratePowNum);
          //  cout<<"Orignal binW: "<<itri_binW<<endl;
          //  cout<<"New BinW:  "<<gsrc_binW<<endl;
            //=============================//
            bool leave= false;
            for(int j=0 ; j< choose ; j++) //number of power
            {
                vector <double> Pd;
                vector < pair<double, double > > power_scaled;

                int min_pd=100;
                double min_power=1000;


                for(int z=0; z< number_blocks ; z++ ) //number of blocks
                {
                    double power ;
                    fin>> power ;
                    if(fin.eof())
                    {
                        leave= true;
                        break;
                    }
                    if(power < 0)
                        power=0;
                    string name = BlockNameITRI[z];

                    int itri_block_area;
                    map<string, int>::iterator iter1;
                    iter1 = area.find(name);
                    if(iter1 != area.end())
                    {
                        itri_block_area = iter1->second;
                    }
                    else
                    {
                        cout<<"can't find the block name"<<name<<endl;
                        getchar();
                    }

                    pair <double,double> Pinfo;
                    Pinfo.first = power;
                    Pinfo.second = pow(itri_block_area,0.5);
                    //According to the bin w adjust power
                    //double factor =(double) gsrc_binW/itri_binW;
                    //double new_power = power * factor;
                   // cout<<"Power "<<Pinfo.first<<endl;
                    power_scaled.push_back(Pinfo);


                    double Pdensity ;
                    if(power<=0)
                        Pdensity =0;
                    else
                        Pdensity = (double) power / itri_block_area;
                    Pd.push_back(Pdensity);

                    if(power < min_power && power != 0)
                        min_power = power;
                    if(Pdensity < min_pd)
                        min_pd = Pdensity;


                }
                if(leave == true)
                    break;

                // fill
                for(int z=0 ; z< fill_num; z++)
                {
                    pair <double,double> temp;
                    temp.first = 0;
                    temp.second = power_scaled[3].second; //width
                    Pd.push_back(0);
                    power_scaled.push_back(temp);
                }

                //Record Max no.3 Power
                vector <double > Maxhot_p;
                Maxhot_p.resize(hot_spot_num);
                for(int z=0 ; z<hot_spot_num; z++)
                    Maxhot_p[z]= -1;
                for(int z=0; z<number_blocks; z++)
                {

                    for(int w=0; w<hot_spot_num; w++)
                    {

                        if(Maxhot_p[w] <power_scaled[z].first )
                        {
                            Maxhot_p[w ] = power_scaled[z].first;
                            break;
                        }
                    }
                }
                //======Start Produce Power=================//
            //    cout<<"Start Produce Power ..."<<endl;
                int hot_count =0;
                for(int z=1 ; z<= copy_num ; z++)
                {

                    for(int w=0; w< number_blocks+fill_num; w++)
                    {
                        double tp = Pd[w];
                        double tpower = power_scaled[w].first ;
                        if(tp == 0)
                            tp==min_pd;
                        if(tpower == 0) //cold
                        {
                            pair<double,double> temp;
                            int random_enhance = rand()%2+1;
                            if(random_enhance== 1)
                                temp.first= min_power * 0.1;
                            else
                                temp.first=0;



                            //temp.first=0;
                            temp.second=power_scaled[w].second;
                            power_scaled.push_back(temp);


                        }
                        else //warm
                        {
                            pair <double,double> temp;
                            bool flag_hot=false;

                            for(int c=0; c<hot_spot_num; c++)
                            {
                                if(Maxhot_p[c]== tpower)
                                {
                                    flag_hot=true;
                                    break;
                                }

                            }

                            //=========== Random enhance in hot mod =====//
                            /*
                            if(flag_hot == true )
                            {
                                double random_enhance = rand()%100+1;
                                int ran_choose = rand()%5+1;
                                random_enhance= random_enhance * 0.01;
                                double Rp ;
                                if( ran_choose== 1)
                                    Rp = (1+random_enhance+0.2) * min_power;
                                else if (ran_choose == 2)
                                    Rp = (1+random_enhance) * min_power;
                                else if (ran_choose == 3)
                                    Rp = (1+random_enhance  ) * min_power;
                                else
                                    Rp =  min_power;
                                // power_scaled.push_back(Rp);


                                temp.first =Rp;
                            }
                            else
                                temp.first =tpower;*/
                            //============== Fix Power ==================//
                            if(flag_hot == true )
                            {
                                if(hot_count < hot_spot_num)
                                {
                                    temp.first = tpower ;
                                    hot_count++;

                                }
                                else
                                {
                                    temp.first = tpower ;

                                }

                            }
                            else
                                temp.first = tpower ;



                            /**@thisforcopyall */
                            //temp.first = tpower * 10; //if you don't want to random assign the hot module close this
                            /**               */
                            temp.first = temp.first * ampFactor ;
                            temp.second= power_scaled[w].second;
                            power_scaled.push_back(temp);


                        }
                        Pd.push_back( tp);
                    }

                }
                //fill
                if(TargetNum % 20 != 0) //can't divide fill
                {
                    int fillNum = TargetNum%20;
                    for(int w=0; w<fillNum; w++)
                    {
                        pair <double,double> temp;
                        temp.first = 0;
                        temp.second = power_scaled[3].second; //width
                        Pd.push_back(0);
                        power_scaled.push_back(temp);
                    }

                }
                cout<<"Power Number :"<<power_scaled.size()<<endl;
                //======Start Produce Power=================//
                //======Start Produce index =================//
                sort (mod_Nnet.begin(), mod_Nnet.end(), compare);
                vector <int > hot_mod;
                vector <int > mod_order ;
                //======Random Order============================//
                /*
                                for(int w=0; w<hot_spot_num; w++)
                                {
                                    int ran_index=rand()%(GeneratePowNum-1 ) ;
                                    hot_mod.push_back(ran_index);
                                }



                                for(int z=0; z< output_case; z++)
                                {
                                    bool jump=false;
                                    for(int w=0; w<hot_mod.size(); w++)
                                    {
                                        if(z == hot_mod[w])
                                        {
                                            jump=true;
                                            break;
                                        }

                                    }
                                    if(jump)
                                        continue;
                                    else
                                        mod_order.push_back(z);
                                }

                                random_shuffle(mod_order.begin(),mod_order.end());
                */
                //============= Area Order ========================//
                multimap <int, int > Area_Sort;
                for(int z = 0; z<output_case ; z++)
                {
                    map<string, int>::iterator iter1;

                    int block_index = z ;

                    iter1 = manual_area.find(power_trace_generate[block_index].block_name);
                    int block_area=iter1->second;
                    Area_Sort.insert ( make_pair (block_area, block_index)  ) ;
                }


                // getchar();
                for(multimap<int, int>::reverse_iterator iter1= Area_Sort.rbegin(); iter1!= Area_Sort.rend(); ++iter1)
                {
                    int BLockIndex = iter1->second;
                    int BlockArea = iter1->first ;
                    if(hot_mod.size() < hot_spot_num )
                    {
                        hot_mod.push_back( BLockIndex );
                    }
                    else
                        mod_order.push_back(BLockIndex);
                }
                random_shuffle(mod_order.begin(),mod_order.end());
                cout<<"mode >> Area sort + random"<<endl;
                // cout<<"HOt mode size : "<<hot_mod.size()<<endl;
                //  cout<<"Other "<< mod_order.size()<<endl;

                //============= Area Order ========================//
/*
                sort (Pd.begin(),Pd.end() );
                sort (power_scaled.begin(), power_scaled.end(), com);
                cout<<"Generated index Finish.."<<endl;

                ofstream Fout("LookIndex.txt");
                for( int z = 0 ; z < output_case ; z ++)
                {
                    int block_index ;
                    if(z>=0 && z<hot_spot_num ) // hotpsot
                    {
                        block_index = hot_mod[z];
                    }
                    else
                        block_index = mod_order[z-hot_spot_num ];

                    map<string, int>::iterator iter1;
                    iter1 = manual_area.find(power_trace_generate[block_index].block_name);
                    int block_area=iter1->second;
                    Fout<<"Block Index "<< block_index <<" Power : "<<power_scaled[z].first << "Area "<< block_area <<endl;

                }
                getchar();

                Fout.close();*/


                //======Start Assign index =================//
                cout<<"Start Assign Index .."<<endl;


                for(int z=0; z<output_case; z++)
                {
                    // choose the module index
                    int block_index ;
                    if(z>=0 && z<hot_spot_num ) // hotpsot
                    {
                        block_index = hot_mod[z];
                    }
                    else
                        block_index = mod_order[z-hot_spot_num ];


                    //block_index = area_index[ z-hot_spot_num ];

                    // block_index = mod_Nnet[z].mod_index; //netlist degree
                    // assign power to module
                    map<string, int>::iterator iter1;
                    int block_area;
                    iter1 = manual_area.find(power_trace_generate[block_index].block_name);
                    if(iter1 != manual_area.end())
                    {

                        block_area = iter1->second;
                    }
                    else
                    {
                        cout<<"can't find the block name"<<power_trace_generate[block_index].block_name<<" index "<<block_index<<" z-h "<<z-hot_spot_num<<endl;
                        getchar();
                    }

                    if(z==0) // amp the no.1 hotspot
                    {
                        double gsrc_W = pow(block_area,0.5);
                        double itri_W = power_scaled[z].second;
                        double factor = (double) gsrc_W/itri_W;
                        double adjust_power = power_scaled[z].first * factor;


                        power_trace_generate[block_index].power.push_back(adjust_power );
                        power_trace_generate[block_index].Pdensity.push_back( (double) adjust_power/ block_area );

                    }
                    else
                    {
                        double gsrc_W = pow(block_area,0.5);
                        double itri_W = power_scaled[ z].second;
                        double factor = (double) gsrc_W/itri_W;
                        double adjust_power = power_scaled[ z].first * factor;

                        power_trace_generate[block_index].power.push_back(adjust_power );
                        power_trace_generate[block_index].Pdensity.push_back( (double) adjust_power/ block_area );


                    }


                }




                fin>>trash;

            }
            break;



        }


        cout<<"Start Assign Power ..."<<endl;
        //=========Assign Power to Block============//
        map<string,POWER>::iterator it1;
        for(int j=0; j< GeneratePowNum ; j++)
        {
            string block_name = power_trace_generate[j].block_name;
            it1=DP_generate.find(block_name);
            if(it1!= DP_generate.end())
            {
                if(it1->second.power.size()==0)
                {
                    it1->second.power = power_trace_generate[j].power;
                    it1->second.Pdensity = power_trace_generate[j].Pdensity;
                }

                else
                {
                    int index = it1->second.power.size()-1;
                    it1->second.power.insert(it1->second.power.end(),power_trace_generate[j].power.begin(),power_trace_generate[j].power.end()); // insert the new power vector in the end of orignal vector
                    it1->second.Pdensity.insert(it1->second.Pdensity.end(),power_trace_generate[j].Pdensity.begin(),power_trace_generate[j].Pdensity.end()); // insert the new power vector in the end of orignal vector
                    // cout<<"nonempty"<<endl;
                }

            }

            else
            {
                cout <<"error : can't find the macro name "<<block_name<<endl;
                getchar();
            }

        }
        fin.close();
        int index = it1->second.power.size()-1;
        switch_index.push_back(index);




    }
    // Output FILE
    string path = "./GeneratedPower/";
    string Out_file = "out";
    for(int i=1; i<=case_num ; i++)
    {
        stringstream s2;
        string label ;
        s2<<i;
        s2>>label;
        string file_out = path+Out_file+label+".pt";
        string read_me = path+"readme"+label+".txt";
        ofstream fout(file_out.c_str());
        ofstream fout_readme(read_me.c_str());
        fout<<"Power unit mW ; "<<endl;
        fout<<"number of block : "<<output_case<<" ;"<<endl;
        fout<<"block : ";
        for (int j=0; j< GeneratePowNum ; j++ )
        {
            fout<<BlockNameGSRC[j]<<" ";
        }
        fout<<"  "<<endl<<endl;
        fout<<"power : "<<endl;
        map<string,POWER>::iterator iter1;
        iter1= DP_generate.begin();
        int power_size= iter1->second.power.size();
        for(int j = switch_index[i-1]; j< switch_index[i]; j++)
        {

            multimap<double,string> sort_power;
            for(int z=0; z< output_case ; z++)
            {
                string out_block_name = BlockNameGSRC[z]; // z choose block
                iter1= DP_generate.find(out_block_name);
                fout<<iter1->second.power[j]<<" "; // j choose time


                map<string, int>::iterator it1;
                int block_area;
                it1 = manual_area.find(out_block_name);
                if(it1 != manual_area.end())
                {

                    block_area = it1->second;
                }
                else
                {
                    cout<<"can't find the block name"<<out_block_name<<endl;
                    getchar();
                }
                double pdd = iter1->second.power[j]  ;
                sort_power.insert(pair<double,string> (pdd,out_block_name));

            }
            multimap<double,string>::iterator it1;
            fout_readme<<"form small to big power : "<<endl;
            int cot =0;
            for(it1=sort_power.begin(); it1!= sort_power.end(); it1++)
            {
                fout_readme<< it1->second<<"  power "<<it1->first<<endl;
                // cout<<"block : "<< it1->second<<" p "<<it1->first<<endl;
                cot++;
            }
            // cout<<"size "<<sort_power.size()<<" count "<<cot<<endl;
            //getchar();
            fout_readme<<endl;
            fout<<" ;";

            fout<<endl;

        }

        fout.close();
        fout_readme.close();
    }






}





int main(int argc,char* argv[])
{
    map <string, int > ITRIArea ;
    map <string, int > GSRCArea ;
    string GeneratedFile ;
    string InputPath;
    int casenum;
    int ampFactor = 1;
    int dy_time = 10 ;
    for(int i=0; i<argc; i++)
    {
        string temp = argv[i];
        if(temp=="-filename")
        {
            string name=argv[i+1];
            GeneratedFile=name;
        }
        if(temp=="-casenum")
        {
            casenum=atoi(argv[i+1]);
        }
        if(temp == "-amp")
            ampFactor = atoi(argv[i+1]) ;
        if(temp == "-dy")
            dy_time = atoi(argv[i+1]) ;
        if(temp== "-datapath")
        {
            string name=argv[i+1];
            InputPath=name;

        }

    }

    string InputFile = "ITRI1";
    cout<<"######  Start Produce Power ######"<<endl;
    cout<<"--------  Input Info  ------------"<<endl;
    cout<<"-File : "<< GeneratedFile<<endl;
    cout<<"-CaseNum: "<<casenum<<endl;
    cout<<"-DataPath : "<<InputPath<<endl;
    cout<<"-Amp : "<<ampFactor<<endl;
    cout<<"-dy : "<<dy_time<<endl<<endl;

    read_block(InputFile,ITRIArea,0);
    read_block(GeneratedFile,GSRCArea,1);
    read_net(GeneratedFile,GSRCArea);


    cout<<"Block Parser End.."<<endl;

    GeneratePower(GSRCArea.size(), ITRIArea, GSRCArea,casenum,ampFactor,InputPath,dy_time);



    return 0;
}
