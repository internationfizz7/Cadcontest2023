/*!
 * \file	fmPartition.cpp
 * \brief	fm partition
 *
 * \author	NCKU SEDA LAB WYChang
 * \version	0.1
 * \date	2018.09.13
 */

#include "fmPartition.h"

#define Abs(x) (x)>=0?(x):-(x)



void initial_gain( NETLIST &nt , int tier );
void Bucket_List( NETLIST &nt , int tier );
void Thermal_bucket_list( NETLIST &nt , int tier );
void Update_gain( NETLIST &nt , int i , int *modarea );
void Thermal_update_gain( NETLIST &nt , int tier , int *modarea);
void InitialThermalGain(NETLIST &nt, int tier);
void InitialTotalGain(NETLIST &nt, int tier);
void CalculatePowerDensity( NETLIST &nt );

bool cmp(int flag, int g, int area);
void Insert( gain *A , gain *B );
void Delete( gain *A );

static bool cmplowerIntInt(pair<int,int> a, pair <int,int> b){
			return a.second < b.second;
}
void InitialThermalGain(NETLIST &nt, int tier)
{
    vector< pair<int,int> > hot_id_gain_pair;
    for(int i=0; i<nt.num_mod; i++)
        nt.mods[ i ].thermal_gain = 0;

    hot_id_gain_pair.reserve(nt.nHot);
    for(int i=0; i<nt.num_mod; i++)
    {
        if(!nt.mods[i].hot_flag)
            continue;
        pair<int, int> temp_pair;

        ///===gain form total hot module of net ===///
        int temp_hot_amount=0;

        for(int j=0; j<nt.mods[i].num_net; j++)
            temp_hot_amount+=nt.nets[ nt.mods[i].array_net[j] ].num_hot;
        ///========================================///
        ///===gain form total power of hot module===///
        //double temp_power_double = nt.mods[i].pd*nt.mods[i].area;
        //int total_power = temp_power_double > (double)INT_MAX? INT_MAX : (int)(nt.mods[i].pd*nt.mods[i].area);
        ///=========================================///

        temp_pair.first = i;
        temp_pair.second = temp_hot_amount; ///<hot module amount
        //temp_pair.second = total_power;///<total power
        hot_id_gain_pair.push_back(temp_pair);
    }

    sort(hot_id_gain_pair.begin(),hot_id_gain_pair.end(),cmplowerIntInt);

    int temp_gain = INT_MIN;
    int temp_new_gain = 0;
    double update_gain_constraint = 0.2;

    for(int i=0; i<nt.nHot; i++)
    {
        if(hot_id_gain_pair[i].second > (int)ceil(temp_gain*(1+update_gain_constraint)) || temp_new_gain == 0)
            temp_new_gain++;
        if(hot_id_gain_pair[i].second > temp_gain)
            temp_gain = hot_id_gain_pair[i].second;

        hot_id_gain_pair[i].second = temp_new_gain;
    }
    for(unsigned int i=0; i<hot_id_gain_pair.size(); i++)
    {
        nt.mods[hot_id_gain_pair[i].first].thermal_gain=hot_id_gain_pair[i].second*3;
        //cout << hot_id_gain_pair[i].first << " " <<nt.mods[hot_id_gain_pair[i].first].thermal_gain << endl;
    }
    maxhotgain = hot_id_gain_pair.back().second;
    hottest_mod_id = hot_id_gain_pair.back().first;
    //cout << hottest_mod_id << " " << maxhotgain << endl;
}

void initial_gain( NETLIST &nt , int tier )
{
    for(int i=0;i<nt.num_net;i++)
    {
        int count1=0;
        int count2=0;
        for( int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++ )
        {
            if( nt.pins[j].is_pad == 1 && (tier==0) )
            {
                count1++;
            }
            else
            {
                if(nt.mods[ nt.pins[j].corr_id ].tier==tier)
                    count1++;
                else if(nt.mods[ nt.pins[j].corr_id ].tier==(tier+1))
                    count2++;
            }
        }
        nt.nets[i].group1_count=count1;
        nt.nets[i].group2_count=count2;

        for(int j = nt.nets[i].head; j < nt.nets[i].head + nt.nets[i].degree; j++)
        {
            if(nt.pins[j].is_pad ==0)
            {
                if(nt.mods[ nt.pins[j].corr_id ].tier==tier)
                {
                    if(nt.nets[i].group1_count==1)
                        nt.mods[ nt.pins[j].corr_id ].gain++;
                    if(nt.nets[i].group2_count==0)
                        nt.mods[ nt.pins[j].corr_id ].gain--;
                }
                else if(nt.mods[ nt.pins[j].corr_id ].tier==(tier+1))
                {
                    if(nt.nets[i].group1_count==0)
                        nt.mods[ nt.pins[j].corr_id ].gain--;
                    if(nt.nets[i].group2_count==1)
                        nt.mods[ nt.pins[j].corr_id ].gain++;
                }
            }
        }
    }
}

void InitialTotalGain(NETLIST &nt, int tier)
{
    ///====determine the hot tier===///
    int hot_tier, cold_tier;
    hot_tier = nt.tierpd_vector[tier]>nt.tierpd_vector[tier+1]? tier : tier+1;
    cold_tier = nt.tierpd_vector[tier]>nt.tierpd_vector[tier+1]? tier+1 : tier;


    for(int i=0;i<nt.num_mod;i++)
    {
        if(nt.mods[i].tier == hot_tier && nt.mods[i].hot_flag)
        {
            nt.mods[i].total_gain = nt.mods[i].gain + nt.mods[i].thermal_gain;
            //cout << "hot tier  " << nt.mods[i].total_gain << endl;
        }
        else if(nt.mods[i].tier == cold_tier && !nt.mods[i].hot_flag)
        {
            nt.mods[i].total_gain = nt.mods[i].gain;
            //cout << "cold tier " << nt.mods[i].total_gain << endl;
        }
    }
}

void Insert( gain *A , gain *B )
{
    if(A->next==NULL)
    {
        A->next=B;
        B->prev=A;
        B->next=NULL;
    }
    else
    {
        B->next=A->next;
        A->next->prev=B;
        A->next=B;
        B->prev=A;
    }
}

void Delete( gain *A )
{
    if(A->next==NULL)
    {
        A->prev->next=NULL;
    }
    else
    {
        A->prev->next=A->next;
        A->next->prev=A->prev;
    }
}

void Bucket_List( NETLIST &nt , int tier )
{
    cout<<"creating bucket"<<endl;
    for(int i=0;i<2*maxpin+1;i++)
    {
        gain *temp1=new gain;
        temp1->cell_name=i-maxpin;
        temp1->next=NULL;
        temp1->prev=NULL;
        List.push_back(temp1);
    }
    for(int i=0;i<nt.num_mod;i++)
    {
        gain *temp=new gain;
        temp->cell_name=nt.mods[i].id;
        temp->next=NULL;
        temp->prev=NULL;
        store_gain.push_back(temp);
    }
    //sort(store_gain.begin(),store_gain.end(),descending)
    for(vector<gain*>::iterator iter=store_gain.begin();iter!=store_gain.end();iter++)
    {
        Insert(List[nt.mods[(*iter)->cell_name].gain+maxpin],(*iter));
    }

    for(int i=0;i<nt.num_mod;i++)
    {
        if(nt.mods[i].num_net==0)
        {
            Delete(store_gain[nt.mods[i].id]);
            nt.mods[i].lock=1;
            //cout<<i<<"  num_net 0"<<endl;
        }
        else if(nt.mods[i].lock==1)
        {
            nt.mods[i].lock=1;
            //cout<<i<<"  lock"<<endl;
        }
        else if(nt.mods[i].hot_flag)
        {
            Delete(store_gain[nt.mods[i].id]);
            nt.mods[i].lock=1;
        }
    }
}

void Thermal_bucket_list( NETLIST &nt , int tier )
{
    cout<<"creating bucket"<<endl;
    for(int i=0;i<2*maxpin+maxhotgain+1;i++)
    {
        gain *temp1=new gain;
        temp1->cell_name=i-maxpin;
        temp1->next=NULL;
        temp1->prev=NULL;
        List.push_back(temp1);
    }
    for(int i=0;i<nt.num_mod;i++)
    {
        gain *temp=new gain;
        temp->cell_name=nt.mods[i].id;
        temp->next=NULL;
        temp->prev=NULL;
        store_gain.push_back(temp);
    }
    //sort(store_gain.begin(),store_gain.end(),descending)
    for(vector<gain*>::iterator iter=store_gain.begin();iter!=store_gain.end();iter++)
    {
        Insert(List[nt.mods[(*iter)->cell_name].total_gain+maxpin+maxhotgain],(*iter));
    }

    for(int i=0;i<nt.num_mod;i++)
    {
        if(nt.mods[i].num_net==0)
        {
            Delete(store_gain[nt.mods[i].id]);
            nt.mods[i].lock=1;
            //cout<<i<<"  num_net 0"<<endl;
        }
        else if(nt.mods[i].lock==1)
        {
            nt.mods[i].lock=1;
            //cout<<i<<"  lock"<<endl;
        }

        //if(nt.tierpd_vector[tier]>nt.tierpd_vector[tier+1])
        if(nt.tierpd_vector[tier]>target_pd[tier])
        {
            if(nt.mods[i].tier == tier && !nt.mods[i].hot_flag)
            {
                Delete(store_gain[nt.mods[i].id]);
                nt.mods[i].lock=1;
            }
            if(nt.mods[i].tier == tier+1 && nt.mods[i].hot_flag)
            {
                //cout << nt.mods[i].tier << " " << nt.mods[i].id << " " << nt.mods[i].hot_flag << endl;
                Delete(store_gain[nt.mods[i].id]);
                nt.mods[i].lock=1;
            }

        }
        //else if(nt.tierpd_vector[tier]<nt.tierpd_vector[tier+1])
        else if(nt.tierpd_vector[tier]<target_pd[tier])
        {
            if(nt.mods[i].tier == tier && nt.mods[i].hot_flag)
            {
                //cout << nt.mods[i].tier << " " << nt.mods[i].id << " " << nt.mods[i].hot_flag << endl;
                Delete(store_gain[nt.mods[i].id]);
                nt.mods[i].lock=1;
            }
            if(nt.mods[i].tier == tier+1 && !nt.mods[i].hot_flag)
            {
                Delete(store_gain[nt.mods[i].id]);
                nt.mods[i].lock=1;
            }

        }

    }

}

bool cmp(int flag, int g, int area)
{
    if(area<mod_constraint)
    {
        if(flag==0)
        {
            if(g>=0)
            {
                return true;
            }
            else
            {
                if(area>g*TSV_AREA)
                    return true;
                else
                    return false;
            }
        }
        else
        {
            return true;

        }
    }
    else
        return false;
}

void Thermal_update_gain( NETLIST &nt , int tier , int *modarea)
{
    int hot_tier, cold_tier;
    int hot_module_count = 0;
    bool end_flag = false;
    bool find_cold_flag = false;
    //hot_tier = nt.tierpd_vector[tier]>nt.tierpd_vector[tier+1]? tier : tier+1;
    //cold_tier = nt.tierpd_vector[tier]>nt.tierpd_vector[tier+1]? tier+1 : tier;
    hot_tier = nt.tierpd_vector[tier]>target_pd[tier]? tier : tier+1;
    cold_tier = nt.tierpd_vector[tier]>target_pd[tier]? tier+1 : tier;
    //cout << "hot_tier " << hot_tier<< endl;

    int num_hot_module = 0;
    for(int i=0; i<nt.num_mod; i++)
    {
        if(nt.mods[i].hot_flag && nt.mods[i].tier == hot_tier)
            num_hot_module++;
    }
    /*cout << num_hot_module << endl;
    getchar();*/

    while(!end_flag)
    {
        for(int i=List.size()-1;i>=0;i--)
        {
            if(List[i]->next!=NULL)
            {
                gain *ptr=List[i]->next;
                while(ptr!=NULL)
                {
                    //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                    //cout << nt.mods[ptr->cell_name].hot_flag << endl;

                    /*if(nt.mods[ptr->cell_name].tier!=tier && nt.mods[ptr->cell_name].tier!=tier+1)
                    {
                        ptr=ptr->next;
                        Delete(store_gain[ptr->prev->cell_name]);
                        continue;
                    }*/
                    if(ptr->cell_name == hottest_mod_id && nt.mods[ptr->cell_name].pd > nt.pd_mean+5*nt.pd_stddevi)
                    {
                        /*cout << "hottest" << endl;
                        cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                        cout << hot_module_count << " " << num_hot_module << endl;*/

                        if(ptr->next!=NULL)
                        {
                            ptr=ptr->next;
                            Delete(store_gain[ptr->prev->cell_name]);
                        }
                        else
                        {
                            Delete(store_gain[ptr->cell_name]);
                            ptr=NULL;
                        }
                        //cout << store_gain.size() << " " << ptr->prev->cell_name << endl;

                        continue;
                    }
                    if(nt.mods[ptr->cell_name].hot_flag && nt.mods[ptr->cell_name].tier == hot_tier)
                    {
                        hot_module_count++;
                    }

                    if(!nt.mods[ptr->cell_name].hot_flag && modarea[hot_tier]>modarea[cold_tier]*1.1)
                    {
                        if(ptr->next!=NULL)
                            ptr=ptr->next;
                        else
                            ptr=NULL;
                        continue;
                    }

                    if((nt.mods[ptr->cell_name].tier==tier+1) && (nt.mods[ptr->cell_name].hot_flag ||
                    (modarea[tier+1]>modarea[tier]*0.9 && cmp(1,abs(nt.mods[ptr->cell_name].gain),nt.mods[ptr->cell_name].area))))        ///from up to down
                    {
                        //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                        nt.mods[ptr->cell_name].lock=1;
                        nt.mods[ptr->cell_name].tier=tier;
                        modarea[tier]=modarea[tier]+nt.mods[ptr->cell_name].area-nt.mods[ptr->cell_name].gain*TSV_AREA;
                        modarea[tier+1]=modarea[tier+1]-nt.mods[ptr->cell_name].area;
                        for(int iteri=0;iteri<nt.mods[ptr->cell_name].num_net;iteri++)
                        {
                            int x=nt.mods[ptr->cell_name].array_net[iteri];
                            for(int iterj=nt.nets[x].head;iterj<nt.nets[x].head+nt.nets[x].degree;iterj++)
                            {
                                if(nt.pins[iterj].is_pad==0 && nt.mods[nt.pins[iterj].corr_id].lock!=1)
                                {
                                    Delete(store_gain[nt.pins[iterj].corr_id]);
                                    if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                    {
                                        if(nt.nets[x].group1_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                        if(nt.nets[x].group2_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                    }
                                    else
                                    {
                                        if(nt.nets[x].group1_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                        if(nt.nets[x].group2_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                    }
                                    nt.nets[x].group1_count++;
                                    nt.nets[x].group2_count--;
                                    if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                    {
                                        if(nt.nets[x].group1_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                        if(nt.nets[x].group2_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                    }
                                    else
                                    {
                                        if(nt.nets[x].group1_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                        if(nt.nets[x].group2_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                    }
                                    Insert(List[nt.mods[nt.pins[iterj].corr_id].total_gain+maxpin+maxhotgain],store_gain[nt.pins[iterj].corr_id]);
                                    nt.nets[x].group1_count--;
                                    nt.nets[x].group2_count++;
                                }
                            }
                            nt.nets[x].group1_count++;
                            nt.nets[x].group2_count--;
                        }
                        Delete(store_gain[ptr->cell_name]);
                        //cout << "Delete " <<ptr->cell_name << endl;

                        CalculatePowerDensity(nt);
                        //if(nt.tierpd_vector[hot_tier]<=nt.tierpd_vector[cold_tier]*1.05 || hot_module_count == num_hot_module-1)
                        if(nt.tierpd_vector[tier]<=target_pd[tier]*1.05 || hot_module_count >= num_hot_module-1)
                        {
                            end_flag = true;
                            break;
                        }
                        else if(nt.mods[ptr->cell_name].hot_flag)
                        {
                            find_cold_flag = true;
                            break;
                        }
                        else
                        {
                            ptr=ptr->next;
                            continue;
                        }
                    }
                    else if((nt.mods[ptr->cell_name].tier==tier) && (nt.mods[ptr->cell_name].hot_flag||
                    (modarea[tier]>modarea[tier+1]*0.9 && cmp(0,abs(nt.mods[ptr->cell_name].gain),nt.mods[ptr->cell_name].area))))
                    {
                        //cout<<"2   "<<nt.mods[ptr->cell_name].area<<endl;
                        //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                        nt.mods[ptr->cell_name].lock=1;
                        nt.mods[ptr->cell_name].tier=tier+1;
                        modarea[tier]=modarea[tier]-nt.mods[ptr->cell_name].area-nt.mods[ptr->cell_name].gain*TSV_AREA;
                        modarea[tier+1]=modarea[tier+1]+nt.mods[ptr->cell_name].area;
                        for(int iteri=0;iteri<nt.mods[ptr->cell_name].num_net;iteri++)
                        {
                            int x=nt.mods[ptr->cell_name].array_net[iteri];
                            for(int iterj=nt.nets[x].head;iterj<nt.nets[x].head+nt.nets[x].degree;iterj++)
                            {
                                if(nt.pins[iterj].is_pad==0 && nt.mods[nt.pins[iterj].corr_id].lock!=1)
                                {
                                    Delete(store_gain[nt.pins[iterj].corr_id]);
                                    if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                    {
                                        if(nt.nets[x].group1_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                        if(nt.nets[x].group2_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                    }
                                    else
                                    {
                                        if(nt.nets[x].group1_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                        if(nt.nets[x].group2_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                    }
                                    nt.nets[x].group1_count--;
                                    nt.nets[x].group2_count++;
                                    if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                    {
                                        if(nt.nets[x].group1_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                        if(nt.nets[x].group2_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                    }
                                    else
                                    {
                                        if(nt.nets[x].group1_count==0)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain--;
                                            nt.mods[nt.pins[iterj].corr_id].gain--;
                                        }
                                        if(nt.nets[x].group2_count==1)
                                        {
                                            nt.mods[nt.pins[iterj].corr_id].total_gain++;
                                            nt.mods[nt.pins[iterj].corr_id].gain++;
                                        }
                                    }
                                    Insert(List[nt.mods[nt.pins[iterj].corr_id].total_gain+maxpin+maxhotgain],store_gain[nt.pins[iterj].corr_id]);
                                    nt.nets[x].group1_count++;
                                    nt.nets[x].group2_count--;
                                }
                            }
                            nt.nets[x].group1_count--;
                            nt.nets[x].group2_count++;
                        }
                        Delete(store_gain[ptr->cell_name]);
                        //cout << "Delete " <<ptr->cell_name << endl;
                        //break;
                        CalculatePowerDensity(nt);
                        //if(nt.tierpd_vector[hot_tier]<=nt.tierpd_vector[cold_tier]*1.05 || hot_module_count >= num_hot_module-1)
                        if(nt.tierpd_vector[tier]<=target_pd[tier]*1.05 || hot_module_count >= num_hot_module-1)
                        {
                            //cout << "break " << hot_module_count << " " << num_hot_module << endl;
                            end_flag = true;
                            break;
                        }
                        else if(nt.mods[ptr->cell_name].hot_flag)
                        {
                            find_cold_flag = true;
                            break;
                        }
                        else
                        {
                            ptr=ptr->next;
                            continue;
                        }
                    }
                    ptr=ptr->next;
                    //if(nt.tierpd_vector[hot_tier]<=nt.tierpd_vector[cold_tier]*1.05 || hot_module_count >= num_hot_module-1)
                    if(nt.tierpd_vector[tier]<=target_pd[tier]*1.05 || hot_module_count >= num_hot_module-1)
                    {
                        //cout << "break " << hot_module_count << " " << num_hot_module << endl;
                        end_flag = true;
                        break;
                    }
                }

            }
            //if(nt.tierpd_vector[hot_tier]<=nt.tierpd_vector[cold_tier]*1.05 || end_flag)
            if(nt.tierpd_vector[tier]<=target_pd[tier]*1.05 || end_flag)
            {
                end_flag = true;
                break;
            }
            else if(find_cold_flag)
            {
                //cout << "break " << hot_module_count << " " << num_hot_module << endl;
                find_cold_flag = false;
                break;
            }

        }
    }
    //getchar();
}

void Update_gain( NETLIST &nt , int tier , int *modarea)
{
    for(int i=List.size()-1;i>=0;i--)
    {
        if(List[i]->next!=NULL)
        {
            gain *ptr=List[i]->next;
            while(ptr!=NULL)
            {
                if(nt.mods[ptr->cell_name].hot_flag)
                {
                    ptr=ptr->next;
                    continue;
                }
                //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                if((nt.mods[ptr->cell_name].tier==tier+1) && cmp(1,abs(nt.mods[ptr->cell_name].gain),nt.mods[ptr->cell_name].area))        ///from up to down
                {
                    if(modarea[tier]>targetArea && List[i]->cell_name<0)
                    {
                        ptr=ptr->next;
                        continue;
                    }
                    else if(modarea[tier]>targetArea*1.01 && List[i]->cell_name>0)
                    {
                        ptr=ptr->next;
                        continue;
                    }
                    //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                    nt.mods[ptr->cell_name].lock=1;
                    nt.mods[ptr->cell_name].tier=tier;
                    modarea[tier]=modarea[tier]+nt.mods[ptr->cell_name].area-nt.mods[ptr->cell_name].gain*TSV_AREA;
                    modarea[tier+1]=modarea[tier+1]-nt.mods[ptr->cell_name].area;
                    for(int iteri=0;iteri<nt.mods[ptr->cell_name].num_net;iteri++)
                    {
                        int x=nt.mods[ptr->cell_name].array_net[iteri];
                        for(int iterj=nt.nets[x].head;iterj<nt.nets[x].head+nt.nets[x].degree;iterj++)
                        {
                            if(nt.pins[iterj].is_pad==0 && nt.mods[nt.pins[iterj].corr_id].lock!=1)
                            {
                                Delete(store_gain[nt.pins[iterj].corr_id]);
                                if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                {
                                    if(nt.nets[x].group1_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                    if(nt.nets[x].group2_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                }
                                else
                                {
                                    if(nt.nets[x].group1_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                    if(nt.nets[x].group2_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                }
                                nt.nets[x].group1_count++;
                                nt.nets[x].group2_count--;
                                if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                {
                                    if(nt.nets[x].group1_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                    if(nt.nets[x].group2_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                }
                                else
                                {
                                    if(nt.nets[x].group1_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                    if(nt.nets[x].group2_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                }
                                Insert(List[nt.mods[nt.pins[iterj].corr_id].gain+maxpin],store_gain[nt.pins[iterj].corr_id]);
                                nt.nets[x].group1_count--;
                                nt.nets[x].group2_count++;
                            }
                        }
                        nt.nets[x].group1_count++;
                        nt.nets[x].group2_count--;
                    }
                    Delete(store_gain[ptr->cell_name]);
                    //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                    //cout << targetArea*1.01 << " " << modarea[tier] << " " << targetArea*0.99 << endl;
                    //break;
                    if((modarea[tier]<=targetArea*1.01)&&(modarea[tier]>=targetArea*0.99))
                        break;
                    else
                    {
                        ptr=ptr->next;
                        continue;
                    }
                }
                else if((nt.mods[ptr->cell_name].tier==tier) && cmp(0,abs(nt.mods[ptr->cell_name].gain),nt.mods[ptr->cell_name].area))
                {
                    if(modarea[tier]<targetArea && List[i]->cell_name<0)
                    {
                        ptr=ptr->next;
                        continue;
                    }
                    else if(modarea[tier]<targetArea*0.99 && List[i]->cell_name>0)
                    {
                        ptr=ptr->next;
                        continue;
                    }
                    //cout<<"2   "<<nt.mods[ptr->cell_name].area<<endl;
                    //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                    nt.mods[ptr->cell_name].lock=1;
                    nt.mods[ptr->cell_name].tier=tier+1;
                    modarea[tier]=modarea[tier]-nt.mods[ptr->cell_name].area-nt.mods[ptr->cell_name].gain*TSV_AREA;
                    modarea[tier+1]=modarea[tier+1]+nt.mods[ptr->cell_name].area;
                    for(int iteri=0;iteri<nt.mods[ptr->cell_name].num_net;iteri++)
                    {
                        int x=nt.mods[ptr->cell_name].array_net[iteri];
                        for(int iterj=nt.nets[x].head;iterj<nt.nets[x].head+nt.nets[x].degree;iterj++)
                        {
                            if(nt.pins[iterj].is_pad==0 && nt.mods[nt.pins[iterj].corr_id].lock!=1)
                            {
                                Delete(store_gain[nt.pins[iterj].corr_id]);
                                if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                {
                                    if(nt.nets[x].group1_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                    if(nt.nets[x].group2_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                }
                                else
                                {
                                    if(nt.nets[x].group1_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                    if(nt.nets[x].group2_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                }
                                nt.nets[x].group1_count--;
                                nt.nets[x].group2_count++;
                                if(nt.mods[nt.pins[iterj].corr_id].tier==tier)
                                {
                                    if(nt.nets[x].group1_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                    if(nt.nets[x].group2_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                }
                                else
                                {
                                    if(nt.nets[x].group1_count==0)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain--;
                                    }
                                    if(nt.nets[x].group2_count==1)
                                    {
                                        nt.mods[nt.pins[iterj].corr_id].gain++;
                                    }
                                }
                                Insert(List[nt.mods[nt.pins[iterj].corr_id].gain+maxpin],store_gain[nt.pins[iterj].corr_id]);
                                nt.nets[x].group1_count++;
                                nt.nets[x].group2_count--;
                            }
                        }
                        nt.nets[x].group1_count--;
                        nt.nets[x].group2_count++;
                    }
                    Delete(store_gain[ptr->cell_name]);
                    //cout << List[i]->cell_name << " " << nt.mods[ptr->cell_name].tier << " " << ptr->cell_name << endl;
                    //cout << targetArea*1.01 << " " << modarea[tier] << " " << targetArea*0.99 << endl;
                    if((modarea[tier]<=targetArea*1.01)&&(modarea[tier]>=targetArea*0.99))
                        break;
                    else
                    {
                        ptr=ptr->next;
                        continue;
                    }
                }
                ptr=ptr->next;
            }

        }
        if((modarea[tier]<=targetArea*1.01)&&(modarea[tier]>=targetArea*0.99))
        {
            break;
        }

    }
    //getchar();
}

void CalculatePower( NETLIST &nt )
{
    vector<int> tier_area_vector;
    tier_area_vector.resize(nt.Layer);

    for(int i=0; i<nt.Layer; i++)
    {
        nt.tierp_vector[i]=0;
        tier_area_vector[i]=0;
    }
    for(int i=0; i<nt.num_mod; i++)
        for(int j=0; j<nt.mods[i].nLayer;j++)
        {
            nt.tierp_vector[nt.mods[i].tier+j]+=(nt.mods[i].pd*nt.mods[i].area);
            tier_area_vector[nt.mods[i].tier+j]+=nt.mods[i].area;
        }
}

void CalculatePowerDensity( NETLIST &nt )
{
    vector<int> tier_area_vector;
    tier_area_vector.resize(nt.Layer);

    for(int i=0; i<nt.Layer; i++)
    {
        nt.tierpd_vector[i]=0;
        tier_area_vector[i]=0;
    }
    for(int i=0; i<nt.num_mod; i++)
        for(int j=0; j<nt.mods[i].nLayer;j++)
        {
            nt.tierpd_vector[nt.mods[i].tier+j]+=(nt.mods[i].pd*nt.mods[i].area);
            //cout << nt.mods[i].pd*nt.mods[i].area << endl;
            tier_area_vector[nt.mods[i].tier+j]+=nt.mods[i].area;
        }
    for(int i=0; i<nt.Layer; i++)
    {
        nt.tierpd_vector[i]/=tier_area_vector[i];
        //cout << nt.tierpd_vector[i] << endl;
    }

}
