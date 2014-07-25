

#include <deque>

#include "hpg.hpp"
#include "spline.hpp"


namespace hpg
{
    class Hpg::Impl
    {
    public:
        unsigned int PosFlowCount;            //< number of positive flows in HPG
        unsigned int AdvFlowCount;            //< number of adverse flows in HPG
        std::deque<double> PosFlows;  //< list of flows in HPG
        std::deque<double> AdvFlows;  //< list of flows in HPG
        std::deque<hpgvec> PosValues; //< list of vectors of HP curves
        std::deque<hpgvec> AdvValues; //< list of vectors of HP curves
        hpgvec PosCritical;           //< critical line for positive flow
        hpgvec AdvCritical;           //< critical line for adverse flow
        hpgvec ZeroFlowLine;          //< zero-flow line (for backwater effects)
        hpgvec NormFlowLine;          //< normal-flow line
        double MinPosFlow;            //< minimum positive flow in HPG
        double MaxPosFlow;            //< maximum positive flow in HPG
        double MinAdvFlow;            //< minimum adverse flow in HPG
        double MaxAdvFlow;            //< maximum adverse flow in HPG
        // Splines are stored here.
        std::deque<magnet::math::Spline> SplPosUS_QDS;       //< spline for US = F(Q, DS) for positive flow
        std::deque<magnet::math::Spline> SplAdvUS_QDS;       //< spline for US = F(Q, DS) for adverse flow
        //std::deque<devinlane::Spline<double, double>> SplPosDS_QUS;       //< spline for US = F(Q, DS) for positive flow
        //std::deque<devinlane::Spline<double, double>> SplAdvDS_QUS;       //< spline for US = F(Q, DS) for adverse flow
        std::deque<magnet::math::Spline> SplPosDS_QUS;       //< spline for US = F(Q, DS) for positive flow
        std::deque<magnet::math::Spline> SplAdvDS_QUS;       //< spline for US = F(Q, DS) for adverse flow
        std::deque<magnet::math::Spline> SplPosVol;          //< spline for Volume = F(Q, DS) for positive flow
        std::deque<magnet::math::Spline> SplAdvVol;          //< spline for Volume = F(Q, DS) for adverse flow
		std::deque<magnet::math::Spline> SplPosHf;
		std::deque<magnet::math::Spline> SplAdvHf;
        magnet::math::Spline SplPosCritUS_Q;   //< spline for Q = F_crit(US) for positive flow
        magnet::math::Spline SplAdvCritUS_Q;   //< spline for Q = F_crit(US) for adverse flow
        magnet::math::Spline SplPosCritDS_Q;   //< spline for Q = F_crit(DS) for positive flow
        magnet::math::Spline SplAdvCritDS_Q;   //< spline for Q = F_crit(DS) for adverse flow
        std::deque<magnet::math::Spline>SplPosCritUS_DS;     //< spline for DS = F_crit(US) for positive flow
        std::deque<point> SplPosCritUS_DS_ranges;   //< range of flows for each of the splines in SplPosCritUS_DS
        magnet::math::Spline SplAdvCritUS_DS;  //< spline for DS = F_crit(US) for adverse flow
        int ErrorCode;

        int     nodeID; /**< the Tunnel ID */
        double	dsInvert; /**< downstream channel bottom elevation - used for HPG header */
        bool	dsInvertValid;
        double	usInvert; /**< upstream channel bottom elevation */
        bool	usInvertValid;
        double	dsStation; /**< downstream stationing */
        bool	dsStationValid;
        double	usStation; /**< upstream stationing */
        bool	usStationValid;
        double	slope; /**< channel slope */
        bool	slopeValid;
        double	length; /**< channel length */
        bool	lengthValid;
        double	roughness; /**< channel roughness */
        bool	roughnessValid;
        double	maxDepth; /**< channel diameter */
        bool	maxDepthValid;
        double	unsteadyDepthPct; /**< maximum depth in percentage */
        bool	unsteadyDepthPctValid;

        void CopyInto(const Impl* copy)
        {
            this->dsInvertValid = copy->dsInvertValid;
            this->usInvertValid = copy->usInvertValid;
            this->dsStationValid = copy->dsStationValid;
            this->usStationValid = copy->usStationValid;
            this->slopeValid = copy->slopeValid;
            this->lengthValid = copy->lengthValid;
            this->roughnessValid = copy->roughnessValid;
            this->maxDepthValid = copy->maxDepthValid;
            this->unsteadyDepthPctValid = copy->unsteadyDepthPctValid;
            this->dsInvert = copy->dsInvert;
            this->usInvert = copy->usInvert;
            this->dsStation = copy->dsStation;
            this->usStation = copy->usStation;
            this->slope = copy->slope;
            this->length = copy->length;
            this->roughness = copy->roughness;
            this->maxDepth = copy->maxDepth;
            this->unsteadyDepthPct = copy->unsteadyDepthPct;
            this->nodeID = copy->nodeID;

            // Copy all of the primitives and STL arrays.
            this->AdvFlowCount = copy->AdvFlowCount;
            this->PosFlowCount = copy->PosFlowCount;
            this->AdvFlows = copy->AdvFlows;
            this->PosFlows = copy->PosFlows;
            this->AdvValues = copy->AdvValues;
            this->PosValues = copy->PosValues;
            this->PosCritical = copy->PosCritical;
            this->AdvCritical = copy->AdvCritical;
            this->ErrorCode = copy->ErrorCode;
            this->MinPosFlow = copy->MinPosFlow;
            this->MaxPosFlow = copy->MaxPosFlow;
            this->MinAdvFlow = copy->MinAdvFlow;
            this->MaxAdvFlow = copy->MaxAdvFlow;
        }
    };
}

