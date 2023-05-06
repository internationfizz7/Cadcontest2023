#include "search.h"

void RandomTargetPd(NETLIST &nt, SEARCHINFO &searchInfo)
{

    double *layerPercent = new double[ metis_nparts ]();
    int *layerRandom = new int[ metis_nparts ]();
    int layerRandomSum = 0;
    ///set random


    int rand_max = 100;


    for(int i=0; i<metis_nparts; i++)
    {
        layerRandom[i] = randint(rand_max+1)*(i+1);
        layerRandomSum += layerRandom[i];
    }

    for(int i=0; i<metis_nparts; i++)
    {
        layerPercent[i] = (double)layerRandom[i]/(double)layerRandomSum;
        searchInfo.PdLayerPercent[i] = layerPercent[i];
    }

    //double each_layer_area = (double)nt.totalModArea/(double)nt.Layer;

/*
    target_pd[nt.Layer-1] = (nt.total_power*top_Target)/each_layer_area;
    target_p[nt.Layer-1] = nt.total_power*top_Target;

    for(int i=0; i<metis_nparts-1; i++)
    {
        target_pd[i] = (nt.total_power*( (1-top_Target)/ (double)(nt.Layer-1))) / each_layer_area;
        target_p[i] = nt.total_power*( (1-top_Target)/ (double)(nt.Layer-1));
    }
*/
    /*for(int i=0; i<metis_nparts; i++)
    {
        target_pd[i] = ((double)nt.total_power * layerPercent[i]) / (double)each_layer_area;
        target_p[i] = (double)nt.total_power * layerPercent[i];
    }*/
    cout << "Target pd: " << endl;
    for(int i=0; i<metis_nparts; i++)
        cout << "  Tier " << i << " : " << searchInfo.PdLayerPercent[i] << endl;
}

int randint(int n)
{

    if ((n - 1) == RAND_MAX)
    {
        return rand();
    }
    else
    {
        long end = RAND_MAX / n;
        assert (end > 0L);
        end *= n;

        int r;
        while ((r = rand()) >= end);

        return r % n;
    }
}

double CaculateSearchCost(IRBIN &IRBin, int* TSV_count )
{
    cout << "====Calculate cost====" << endl;
    double cost = 0;
    double temperature_cost = 0;
    double TSV_cost = 0;
    double temperature_avg = 0;
    for(int i=0; i< metis_nparts; i++)
        temperature_avg += IRBin.LayerTemperatureWT[i];

    temperature_avg /= (double)metis_nparts;

    for(int i=0; i< metis_nparts; i++)
        temperature_cost += pow((IRBin.LayerTemperatureWT[i] - temperature_avg)/temperature_avg,2);

    TSV_cost = double(totalTSV-initTsvNum)/(double)initTsvNum;

    cost = TSV_cost+2*temperature_cost;
    cout << "Cost:" << endl;
    cout << "Temperature:\t" << temperature_cost << endl;
    cout << "TSV:\t\t" << TSV_cost << endl;
    cout << "Total:\t\t" << cost << endl;

    return cost;
}

void SetGridSearchRange(SEARCHINFO &searchInfo, GRIDRANGE &gridRange)
{
    gridRange.upperBound = new double[metis_nparts]();
    gridRange.lowerBound = new double[metis_nparts]();

    for(int i=0; i<metis_nparts; i++)
    {
        gridRange.upperBound[i] = (searchInfo.PdLayerPercent[i]+0.1)>1?1:searchInfo.PdLayerPercent[i]+0.1;
        gridRange.lowerBound[i] = (searchInfo.PdLayerPercent[i]-0.1)<0?0:searchInfo.PdLayerPercent[i]-0.1;
        cout << gridRange.upperBound[i] << " "  << gridRange.lowerBound[i]  << endl;
    }
}


void RecursiveGrid(GRIDRANGE &gridRange, int layer, double *layerPdArray, vector<SEARCHINFO> &searchInfoVec)
{
    //cout << layer << endl;

    double sum_pd = 0.0;
    if(layer == metis_nparts-1)
    {
        //cout << "here1" << endl;
        for(int i=0; i<layer; i++)
            sum_pd += layerPdArray[i];

        layerPdArray[layer] = 1-sum_pd;
        //cout << "sum_pd "<< sum_pd << endl;
        //cout << "layerPdArray[layer] "<< layerPdArray[layer] << endl;

        if(layerPdArray[layer]<=gridRange.upperBound[layer] && layerPdArray[layer]>=gridRange.lowerBound[layer])
        {
            SEARCHINFO tempSearchInfo;
            for(int j=0; j<metis_nparts; j++)
                tempSearchInfo.PdLayerPercent[j] = layerPdArray[j];
            searchInfoVec.push_back(tempSearchInfo);
/*
            cout << "new target" << endl;
            for(int j=0; j<metis_nparts; j++)
                cout << j << " : " << tempSearchInfo.PdLayerPercent[j] << endl;
*/
        }
        return;
    }
    else
        layerPdArray[layer] = gridRange.lowerBound[layer];

    double resolution = 0.01;
    double levelRange = gridRange.upperBound[layer]-gridRange.lowerBound[layer];
    //cout << "here2" << endl;
    for(int i=0; i<(int)floor(levelRange/resolution); i++ )
    {
        layerPdArray[layer] = layerPdArray[layer]+resolution;


        for(int j=0; j<layer; j++)
            sum_pd += layerPdArray[j];
        if(sum_pd >= 1)
            return;
        else
            RecursiveGrid(gridRange, layer+1, layerPdArray, searchInfoVec);
    }
}
