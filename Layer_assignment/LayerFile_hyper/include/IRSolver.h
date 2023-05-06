#ifndef _IRSOLVER_H_
#define _IRSOLVER_H_


#include "structure.h"

class IRSOLVER
{
    protected:
        class LAYER;
        class MATERIAL;
        class CROSSNODE;

        double IRWidth;
        double IRHeight;
        double IRArea;

        double realWidth;
        double realHeight;
        double realArea;


        double TSVresistivity;
        double scale;

        int nLayer;
        int nMaterial;
        int nNode;

        vector<LAYER> Layers;
        vector<MATERIAL> Materials;
        vector<CROSSNODE> Nodes;

        map<string, MATERIAL*> NameToMaterial;

        class LAYER
        {
            public:
                string materialName;
                string effect; ///BEOL, active, passive, bond, heatsink, heatspreader, Substrate...
                int id;
                int TSVNum;
                double resistanceWT;
                double resistanceWOT;
                double convection;
                double power;
                double thickness;
                bool layerFlag;///0 for layer, 1 for heatsink PCB and so on.
                CROSSNODE *Node;

                LAYER()
                {
                    materialName = effect="";
                    id = TSVNum = 0;
                    resistanceWT = resistanceWOT = power = thickness = convection = 0.0;
                    layerFlag = false;
                }
                LAYER(MATERIAL& material)
                {
                    effect="";
                    id = TSVNum = 0;
                    resistanceWT = resistanceWOT = power = thickness = 0.0;
                    materialName = material.name;
                    layerFlag = material.layerFlag;
                    convection = material.convection;
                }

        };

        class MATERIAL
        {
            public:
                string name;
                double resistivity;
                double convection;
                bool layerFlag; ///0 for layer, 1 for heatsink PCB and so on.

                MATERIAL()
                {
                    name="";
                    resistivity = convection = 0;
                    layerFlag = false;
                }
                MATERIAL(string Mname, double Mresistivity, double Mconvection, bool MlayerFlag)
                {
                    name = Mname;
                    resistivity = Mresistivity;
                    convection = Mconvection;
                    layerFlag = MlayerFlag;
                }

        };


        class CROSSNODE
        {
            public:
                string materialName;
                //int layerNum;
                int id; /// crossnode's position in list
                double low_conductor, up_conductor;
                double low_resistance, up_resistance;
                double voltage, current_static, conductor;
                double total_I;
                bool visitedFlag;// , non_current;
                bool powerFlag;		/// 0=no connect to active; 1=connect to active
                bool layerFlag; /// 0 for layer, 1 for heatsink PCB and so on.
                CROSSNODE *low_node, *up_node;  /// point to adjacency node
                CROSSNODE()
                {
                    id = -1;
                    total_I  = 0.0;
                    low_node = NULL; up_node = NULL;
                    low_conductor = up_conductor = 0.0;
                    low_resistance = up_resistance = 0.0;
                    voltage = 0.0; current_static = 0.0; conductor = 0.0;
                    visitedFlag = powerFlag=layerFlag= false;
                }
        };

    public:
        void IRmain(IRBIN& bin, int nLayer);
        void IRmain(vector< vector<IRBIN> >& bin_matrix, int nLayer);
        void CalculateTempVariation(IRBIN& bin);
        void CalculatePowerVariation(IRBIN& bin);
        void CalculatePowerVariation(IRBIN& bin, double TSVRatio, vector<double>& PowerVector);
        void CleanStructure();
        void ConstructStructure(bool mode);  ///mode false = with TSV, true = without TSV
        void ConstructMaterial(string Mname, double Mresistivity, double Mconvection, bool MlayerFlag);
        void ConstructLayer(MATERIAL& material, string layer_effect, int TSV_number, double power, double thickness);
        void ConstructLayer(MATERIAL& material, string layer_effect, int TSV_number, double power, double thickness, double side);
        void SetIRSolver(double width, double height);
        void SetIRResult(IRBIN& bin, bool mode);
        void SolveIR();

        IRSOLVER()
        {
            IRWidth = IRHeight = IRArea = 0;
            realWidth = realHeight = realArea = 0.0;
            TSVresistivity = 0.0;

            nLayer = nMaterial = nNode = 0;
            scale = 1000000;
        }
        IRSOLVER(double width, double height)
        {
            IRWidth = width;
            IRHeight = height;
            IRArea = width*height;

            realWidth = (IRWidth/AMP_PARA/this->scale);
            realHeight = (IRHeight/AMP_PARA/this->scale);
            realArea = realWidth*realHeight;

            scale = 1000000;
        }
};

#endif
