#include "IRSolver.h"

/*!
 * \file 	IRSolver.cpp
 * \brief 	Solve the reality temperature in Z direction
 *
 * \author 	Wei-Yi Chang
 * \version	0.1
 * \date	2019.05.
 */

void IRSOLVER::CleanStructure()
{
    nLayer=0;
    nNode=0;

    Layers.clear();
    Nodes.clear();
}

void IRSOLVER::ConstructStructure(bool mode)///mode false = with TSV, true = without TSV
{
    int active_count = 0;
    vector<CROSSNODE> temp_Node_vector;
    temp_Node_vector.reserve(nLayer-1);
    for(int i=0; i<nLayer-1; i++)
    {
        CROSSNODE tempNode;
        //cout << Layers[i].effect << endl;

        tempNode.id = i;
        if(mode == false)
        {
            ///low conductor
            if(Layers[i].layerFlag == false)
                tempNode.low_resistance = Layers[i].resistanceWT;
            else
                tempNode.low_resistance = (Layers[i].resistanceWT+Layers[i].convection);

            ///up conductor
            if(Layers[i+1].layerFlag == false)
                tempNode.up_resistance = Layers[i+1].resistanceWT;
            else
                tempNode.up_resistance = (Layers[i+1].resistanceWT+Layers[i+1].convection);
        }
        else
        {
             ///low conductor
            if(Layers[i].layerFlag == false)
                tempNode.low_resistance = Layers[i].resistanceWOT;
            else
                tempNode.low_resistance = (Layers[i].resistanceWOT+Layers[i].convection);

            ///up conductor
            if(Layers[i+1].layerFlag == false)
                tempNode.up_resistance = Layers[i+1].resistanceWOT;
            else
                tempNode.up_resistance = (Layers[i+1].resistanceWOT+Layers[i+1].convection);
        }

        //cout << i << " " <<Layers[i+1].resistanceWT << " " << Layers[i+1].convection << endl;

        if(Layers[i].layerFlag || Layers[i+1].layerFlag)
            tempNode.layerFlag = true;


        if(Layers[i+1].effect == "active")
        {
            tempNode.current_static = Layers[i+1].power;
            tempNode.powerFlag = true;
            active_count++;
        }
        tempNode.conductor = tempNode.low_conductor+tempNode.up_conductor;
        temp_Node_vector.push_back(tempNode);

    }

    ///reduce the node
    int active_count2=0;
    Nodes.reserve(active_count+2);
    Nodes.resize(active_count);

    for(unsigned int i=0; i<temp_Node_vector.size(); i++)
    {
        if(active_count2 < active_count)
        {
            Nodes[active_count2].low_resistance += temp_Node_vector[i].low_resistance;
            //cout << temp_Node_vector[i].low_resistance << endl;
            if(temp_Node_vector[i].powerFlag == true)
            {
                Nodes[active_count2].current_static = temp_Node_vector[i].current_static;
                Nodes[active_count2].low_conductor = 1/Nodes[active_count2].low_resistance;
                Nodes[active_count2].id = active_count2+1;
                Nodes[active_count2].powerFlag = true;
                if(active_count2!=0)
                {
                    Nodes[active_count2-1].up_resistance = Nodes[active_count2].low_resistance;
                    Nodes[active_count2-1].up_conductor = Nodes[active_count2].low_conductor;
                }
                Nodes[active_count2].up_resistance =  temp_Node_vector[i].up_resistance; ///for the last node
                //cout << active_count2 << "low: " << Nodes[active_count2].low_resistance << endl;
                active_count2++;

            }
        }
        else
        {
            Nodes[active_count2-1].up_resistance += temp_Node_vector[i].up_resistance;
            //cout <<temp_Node_vector[i].up_resistance<< endl;
            if(i == temp_Node_vector.size()-1)
            {
                Nodes[active_count2-1].up_conductor = 1/Nodes[active_count2-1].up_resistance;
                //cout << active_count2-1 << "up : " << Nodes[active_count2-1].up_resistance << endl;
            }
        }
    }
    for(unsigned int i=0; i<Nodes.size(); i++)
    {
        Nodes[i].conductor = Nodes[i].up_conductor+Nodes[i].low_conductor;
    }
    ///assign ground
    CROSSNODE templowNode;
    CROSSNODE tempupNode;

    templowNode.up_conductor = Nodes.front().low_conductor;
    templowNode.voltage = 0;
    templowNode.layerFlag = true;
    templowNode.id = 0;

    tempupNode.low_conductor = Nodes.back().up_conductor;
    tempupNode.voltage = 0;
    tempupNode.layerFlag = true;
    templowNode.id = Nodes.size();

    Nodes.insert(Nodes.begin(), templowNode);
    Nodes.insert(Nodes.end(), tempupNode);

    ///connect pointer

    for(unsigned int i=0; i<Nodes.size(); i++)
    {
        if(i==0)
        {
            Nodes[i].low_node = (CROSSNODE*)NULL;
            Nodes[i].up_node = &Nodes[i+1];
        }
        else if(i == Nodes.size())
        {
            Nodes[i].low_node = &Nodes[i-1];
            Nodes[i].up_node = (CROSSNODE*)NULL;
        }
        else
        {
            Nodes[i].low_node = &Nodes[i-1];
            Nodes[i].up_node = &Nodes[i+1];
        }
    }
}

void IRSOLVER::ConstructMaterial(string Mname, double Mresistivity, double Mconvection, bool MlayerFlag)
{
    ///check the material vector
    map<string, MATERIAL*>::iterator map_iter;
    map_iter = NameToMaterial.find(Mname);
    if(map_iter != NameToMaterial.end())
    {
        cout << "[Error] Same Material named " << Mname << " !" << endl;
        cout << "[info] Please rename and check" << endl;
        exit(1);
    }

    ///add material

    MATERIAL tempMaterial(Mname, Mresistivity, Mconvection, MlayerFlag);
    Materials.push_back(tempMaterial);
    nMaterial++;
    NameToMaterial[Mname] = &Materials.back();
}

void IRSOLVER::ConstructLayer(MATERIAL& material, string layer_effect, int TSV_number, double power, double thickness)
{
    LAYER tempLayer(material);
    tempLayer.TSVNum    = TSV_number;
    tempLayer.effect    = layer_effect;
    tempLayer.power     = power*0.001;
    tempLayer.thickness = thickness;
    tempLayer.id        = nLayer;
    double real_thickness = (tempLayer.thickness/AMP_PARA/this->scale);
    double TSVRealSize = TSV_SIZE/this->scale;
    double TSVRealArea = TSVRealSize*TSVRealSize;
    double TSVTotalArea = TSVRealArea*TSV_number;

    if(TSV_number==0)
    {
        tempLayer.resistanceWOT = material.resistivity*real_thickness/realArea;
        tempLayer.resistanceWT = material.resistivity*real_thickness/realArea;
    }
    else
    {
        tempLayer.resistanceWOT = material.resistivity*real_thickness/realArea;
        double TSVresistance = TSVresistivity*real_thickness/(TSVTotalArea);
        double Otherresistance;
        if(TSVTotalArea < realArea)
            Otherresistance = material.resistivity*real_thickness/(realArea-TSVTotalArea);
        else
            Otherresistance = 0;
        tempLayer.resistanceWT = (TSVresistance*Otherresistance)/(TSVresistance+Otherresistance);
    }
    //cout << layer_effect << " " << nLayer << endl;
    //cout << tempLayer.resistanceWT << " " << tempLayer.resistanceWOT << endl;

    Layers.push_back(tempLayer);

    nLayer++;
}

void IRSOLVER::ConstructLayer(MATERIAL& material, string layer_effect, int TSV_number, double power, double thickness, double side)
{
    LAYER tempLayer(material);
    tempLayer.TSVNum    = TSV_number;
    tempLayer.effect    = layer_effect;
    tempLayer.power     = power;
    tempLayer.thickness = thickness;
    tempLayer.id        = nLayer;

    double real_side = side/AMP_PARA/this->scale;
    double side_area = real_side * real_side;
    double real_thickness = (tempLayer.thickness/AMP_PARA/this->scale);
    double TSVRealSize = TSV_SIZE/this->scale;
    double TSVRealArea = TSVRealSize*TSVRealSize;
    double TSVTotalArea = TSVRealArea*TSV_number;

    if(TSV_number==0)
    {
        tempLayer.resistanceWOT = material.resistivity*real_thickness/side_area;
        tempLayer.resistanceWT = material.resistivity*real_thickness/side_area;
    }
    else
    {
        tempLayer.resistanceWOT = material.resistivity*real_thickness/side_area;
        double TSVresistance = TSVresistivity*real_thickness/(TSVTotalArea);
        double Otherresistance;
        if(TSVTotalArea < side_area)
            Otherresistance = material.resistivity*real_thickness/(side_area-TSVTotalArea);
        else
            Otherresistance = 0;
        tempLayer.resistanceWT = (TSVresistance*Otherresistance)/(TSVresistance+Otherresistance);
    }
    //cout << layer_effect << " " << nLayer << endl;
    //cout << tempLayer.resistanceWT << " " << tempLayer.resistanceWOT << endl;
    Layers.push_back(tempLayer);

    nLayer++;
}

void IRSOLVER::SetIRSolver(double width, double height)
{
        IRWidth = width;
        IRHeight = height;
        IRArea = width*height;

        realWidth = (IRWidth/AMP_PARA/this->scale);
        realHeight = (IRHeight/AMP_PARA/this->scale);
        realArea = realWidth*realHeight;

        nLayer = nMaterial = nNode = 0;


        ///solver constant
        //scale = 1000000;
        TSVresistivity = 0.0025316;

        ///material info
        double Si_resistivity = 0.00851;
        double BEOL_resistivity = 0.444;
        double Bond_resistivity = 5;
        double heatsink_resistivity = 0.0025;
        double heatspreader_resistivity = 0.0025;
        double PCB_resistivity = 0;

        Materials.reserve(6);

        ConstructMaterial("Si", Si_resistivity, 0, false);
        ConstructMaterial("BEOL", BEOL_resistivity, 0, false);
        ConstructMaterial("Bond", Bond_resistivity, 0, false);
        ConstructMaterial("heatsink", heatsink_resistivity, 0.1, true );
        ConstructMaterial("heatspreader", heatspreader_resistivity, 0, false );
        ConstructMaterial("PCB", PCB_resistivity,5000, true );

}

void IRSOLVER::SolveIR()
{
    bool convergence = false;
    double tmp_V = 0.0;
    double prev_V = 0.0;
    double error = 0.0001;
    CROSSNODE* curr_node;
    while(!convergence)
    {
        convergence = true;
        for(unsigned int i=0; i<Nodes.size(); i++)
        {
            curr_node = &Nodes[i];
            if(!curr_node->layerFlag)
            {
                //cout << "Node: " << i << endl;
                tmp_V = 0.0;
                prev_V = curr_node->voltage;

                if(curr_node->low_node != (CROSSNODE*)NULL)
                {
                    tmp_V = tmp_V+(curr_node->low_conductor/curr_node->conductor)*curr_node->low_node->voltage;
                    //cout << "low :" << curr_node->low_conductor << " " << curr_node->conductor << endl;
                }
                if(curr_node->up_node != (CROSSNODE*)NULL)
                {
                    tmp_V = tmp_V+(curr_node->up_conductor/curr_node->conductor)*curr_node->up_node->voltage;
                    //cout << "up :" << curr_node->up_conductor << " " << curr_node->conductor << endl;
                }

                tmp_V = tmp_V + (curr_node->current_static/curr_node->conductor);

                curr_node->voltage = tmp_V;
                //cout << tmp_V << endl;
                if(fabs(curr_node->voltage - prev_V) >= error)
                    convergence = false;
            }
        }
    }
}

void IRSOLVER::SetIRResult(IRBIN& bin, bool mode)
{
    for(unsigned int i=0; i<bin.LayerPower.size(); i++)
    {
        if(!mode)
            bin.LayerTemperatureWT[i] = Nodes[i+1].voltage;
        else
            bin.LayerTemperatureWOT[i] = Nodes[i+1].voltage;

        //cout << "voltage " << i << " Layer: " << Nodes[i+1].voltage << endl;
    }
    if(bin.simLayer!=-1)
    {
        if(!mode)
            bin.resultWT = Nodes[bin.simLayer].voltage;
        else
            bin.result = Nodes[bin.simLayer].voltage;
    }
}


void IRSOLVER::CalculateTempVariation(IRBIN& bin)
{
    bin.temperatureVariation = bin.result/bin.resultWT;
    /*cout << "temperature variation " << endl;
    cout << bin.temperatureVariation << endl;*/
}

void IRSOLVER::CalculatePowerVariation(IRBIN& bin)
{
    bin.powerVariation = bin.LayerPower[bin.simLayer]*bin.temperatureVariation;
}

void IRSOLVER::CalculatePowerVariation(IRBIN& bin, double TSVRatio, vector<double>& PowerVector)
{
    bin.powerVariation=abs(PowerVector[bin.simLayer-1]*TSVRatio*bin.temperatureVariation);
    /*cout << "power variation" << endl;
    cout << bin.powerVariation << endl;
    cout << "TSV ratio" << endl;
    cout << TSVRatio << endl;*/
}

void IRSOLVER::IRmain(IRBIN& bin, int nLayer)
{
    double IRwidth = (double)bin.width;
    double IRheight = (double)bin.height;

    ///thickness
    double BEOL_thickness = (1.2e-05) * AMP_PARA * this->scale;
    double active_thickness = (2e-06) * AMP_PARA * this->scale;
    double passive_thickness = (4.8e-05) * AMP_PARA * this->scale;
    double bond_thickness = (2e-05) * AMP_PARA * this->scale;
    double spreader_thickness = (0.001) * AMP_PARA * this->scale;
    double sink_thickness = (0.0069) * AMP_PARA * this->scale;

    ///side
    double spreader_side = 0.03 * AMP_PARA * this->scale;
    double sink_side = 0.06 * AMP_PARA * this->scale;

    SetIRSolver(IRwidth, IRheight);

    ///construct layer
    int TSVnum=0;
    Layers.reserve(nLayer*4+2);
    Nodes.reserve(nLayer*4+2);
    ConstructLayer(*NameToMaterial["PCB"], "PCB", 0, 0, 1);

    for(int i=0; i<nLayer; i++)
    {
        ConstructLayer(*NameToMaterial["BEOL"], "BEOL", TSVnum, 0, BEOL_thickness);

        TSVnum = bin.LayerTSV[i];

        ConstructLayer(*NameToMaterial["Si"], "active", TSVnum, bin.LayerPower[i], active_thickness);
        ConstructLayer(*NameToMaterial["Si"], "passive", TSVnum, 0, passive_thickness);
        if(i!=(nLayer-1))
            ConstructLayer(*NameToMaterial["Bond"], "bond", TSVnum, 0, bond_thickness);
    }
    ConstructLayer(*NameToMaterial["heatspreader"], "heatspreader", 0, 0, spreader_thickness, spreader_side);
    ConstructLayer(*NameToMaterial["heatsink"], "heatsink", 0, 0, sink_thickness, sink_side);

    ConstructStructure(false);
    SolveIR();
    SetIRResult(bin, false);
    //getchar();

    /*if(bin.simLayer!=-1)
    {
        Nodes.clear();
        ConstructStructure(true);
        SolveIR();
        SetIRResult(bin, true);
        CalculatePowerVariation(bin);
    }*/

}

void IRSOLVER::IRmain(vector< vector<IRBIN> >& bin_matrix, int nLayer)
{
    double IRwidth = (double)bin_matrix[0][0].width;
    double IRheight = (double)bin_matrix[0][0].height;

    ///thickness
    double BEOL_thickness = (1.2e-05) * AMP_PARA * this->scale;
    double active_thickness = (2e-06) * AMP_PARA * this->scale;
    double passive_thickness = (4.8e-05) * AMP_PARA * this->scale;
    double bond_thickness = (2e-05) * AMP_PARA * this->scale;
    double spreader_thickness = (0.001) * AMP_PARA * this->scale;
    double sink_thickness = (0.0069) * AMP_PARA * this->scale;

    ///side
    double spreader_side = 0.03 * AMP_PARA * this->scale;
    double sink_side = 0.06 * AMP_PARA * this->scale;

    int window_size = 5;
    int bin_enlarge_half = (window_size-1)/2;

    SetIRSolver(IRwidth*window_size, IRheight*window_size);

    for(unsigned int i=0; i<bin_matrix.size(); i++)
    {
        for(unsigned int j=0; j<bin_matrix[i].size(); j++)
        {
            //cout << bin_matrix[i][j].simLayer << endl;
            int simLayer = bin_matrix[i][j].simLayer;
            if(bin_matrix[i][j].LayerTSV[simLayer-1] == 0)
                continue;
            else
                bin_matrix[i][j].simFlag = true;

            CleanStructure();
            ///set windows
            int leftc  = ((int)i-bin_enlarge_half)>0?i-bin_enlarge_half:0;
            int lowr   = ((int)j-bin_enlarge_half)>0?j-bin_enlarge_half:0;
            int rightc = (i+bin_enlarge_half)<bin_matrix.size()?i+bin_enlarge_half:bin_matrix.size()-1;
            int upr    = (j+bin_enlarge_half)<bin_matrix[i].size()?j+bin_enlarge_half:bin_matrix[i].size()-1;
            ///construct layer
            //cout << "bin: " << i << " " << j << endl;
            //cout << bin_matrix[i][j] << endl;
            vector<int> TSVNumVector;
            vector<double> PowerSumVector;
            double TSVRatio;

            TSVNumVector.assign(nLayer, 0);
            PowerSumVector.assign(nLayer, 0.0);

            Layers.reserve(nLayer*4+2);
            Nodes.reserve(nLayer*4+2);
            ConstructLayer(*NameToMaterial["PCB"], "PCB", 0, 0, 1);


            for(int k=0; k<nLayer; k++)
            {
                ConstructLayer(*NameToMaterial["BEOL"], "BEOL", TSVNumVector[k], 0, BEOL_thickness);
                for(int i2=leftc; i2<rightc+1; i2++)
                    for(int j2=lowr; j2<upr+1; j2++)
                    {
                        TSVNumVector[k] += bin_matrix[i2][j2].LayerTSV[k];
                        PowerSumVector[k]+=bin_matrix[i2][j2].LayerPower[k];
                    }

                ConstructLayer(*NameToMaterial["Si"], "active", TSVNumVector[k], PowerSumVector[k], active_thickness);
                ConstructLayer(*NameToMaterial["Si"], "passive", TSVNumVector[k], 0, passive_thickness);
                if(k!=(nLayer-1))
                    ConstructLayer(*NameToMaterial["Bond"], "bond",TSVNumVector[k], 0, bond_thickness);
            }

            /*cout << "Power:" << endl;
            for(int k=0; k<nLayer; k++)
            {
                cout << PowerSumVector[k] << endl;
            }

            cout << "TSV:" << endl;
            for(int k=0; k<nLayer; k++)
            {
                cout << TSVNumVector[k] << endl;
            }*/

            ConstructLayer(*NameToMaterial["heatspreader"], "heatspreader", 0, 0, spreader_thickness, spreader_side);
            ConstructLayer(*NameToMaterial["heatsink"], "heatsink", 0, 0, sink_thickness, sink_side);

            if(PowerSumVector[simLayer-1] == 0 || PowerSumVector[simLayer-1]*40 <PowerSumVector[simLayer])
            {
                bin_matrix[i][j].simFlag = false;
                continue;
            }
            ConstructStructure(true); //without TSV
            SolveIR();
            SetIRResult(bin_matrix[i][j], true);

            if(simLayer!=-1)
            {
                TSVRatio =(double)bin_matrix[i][j].LayerTSV[simLayer-1]/(double)TSVNumVector[simLayer-1];
                Nodes.clear();
                ConstructStructure(false);//with TSV
                SolveIR();
                SetIRResult(bin_matrix[i][j], false);
                CalculateTempVariation(bin_matrix[i][j]);
                if(bin_matrix[i][j].temperatureVariation>1)
                {
                    bin_matrix[i][j].simFlag = false;
                    continue;
                }
                CalculatePowerVariation(bin_matrix[i][j], TSVRatio, PowerSumVector);
            }
            //getchar();
        }
    }
}
