
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <algorithm>

#include "../hpg/error.hpp"
#include "../util/math.h"

#include "hpg_creator.hpp"
#include "benchmark.h"
#include "normcrit.h"
#include "profile.h"


// NOTE: BENCH_* macros will be empty unless BENCHMARKYES is defined
// (usually in project settings)

void HpgCreator::computeHpgCurve(const xs::Reach& reach, double flow, double pressurizedHeight, bool reverseSlope, double& yNormal, double& yCritical, hpg::hpgvec& curve)
{
    BENCH_INIT;

    bool computeFreeOnly = pressurizedHeight < 1e-6;

    //double slope = reach.getSlope();// * (reverseSlope ? -1. : 1.);
    double maxDepth = reach.getMaxDepth();
    double dsInvert = reach.getDsInvert();
    //if (reverseSlope)
    //    dsInvert = reach.getUsInvert();
    double usInvert = reach.getUsInvert();
    //if (reverseSlope)
    //    usInvert = reach.getDsInvert();

    bool yNvalid = false;
    // Normal depth is only defined for positive slope channels.
    if (!reverseSlope && !isZero(flow))
    {
        BENCH_START;
    	
        if (ComputeNormalDepth(reach, flow, this->g, this->kn, yNormal))
        {
            if (computeFreeOnly)
            {
                this->errorCode = hpg::error::divergence;
                return;
            }
        }
        else
        {
            yNvalid = true;
        }
        
        BENCH_STOP;
    }

    BENCH_START;

    bool yCvalid = false;
	if (!isZero(flow))
    {
        if (!ComputeCriticalDepth(reach, flow, this->g, yCritical))
        {
            yCvalid = true;
        }
        else
        {
            //this->errorCode = hpg::error::divergence;
            //return;
        }
    }
        
    BENCH_STOP;


    // isSteep = true if the channel is steep-slope and the slope is positive.
    bool isSteep = (yNvalid && yCvalid && yCritical > yNormal && !reverseSlope);

    // Set the min/max according to the channel characteristics (steep/mild).
    double yMin, yMax;
    if (isSteep)
    {
        // The following two lines would be the case if this was an S2 curve (supercritical flow).
        //yMin = diameter * 0.005; // 0.5% of diameter is the minimum
        //yMax = yCritical;
        // This is the case for S1 curves.
        if (yCvalid)
            yMin = yCritical;
        else
            yMin = maxDepth * 0.01; // 1% of diameter
        yMax = this->maxDepthFrac * maxDepth;
    }
    else
    {
        if (yCvalid)
            yMin = yCritical;
        else
            yMin = maxDepth * 0.01; // 1% of diameter
        yMax = this->maxDepthFrac * maxDepth;
    }

    int np = this->numPoints;

    if (yMax < yMin)
        yMax = yMin;

    if (isZero(yMax - yMin))
    {
        np = 1;
    }
    else
    {
        // Minimum of 
        np = std::min(this->numPoints, std::max(1, (int)std::floor((yMax - yMin) / (0.05 * maxDepth) + 0.5)));
    }

    // Compute the depth increment in vertical direction.
    double dy = (yMax - yMin) / np;

    // If the user specified a pressurized height, then we do two executions of the loop, the first
    // to do the free-surface computations and the second to do the pressurized computations.
    // If the pressurized height is zero, then just do the free-surface computations.
    int freePressLoopActivation = computeFreeOnly ? 1 : 2;
    double dz = 0;
    for (int i = 0; i < freePressLoopActivation; i++)
    {
        // Starting depth.
        double yInit = yMin;
        if (i > 0)
        {
            yInit = yMax + dy;
            yMax += pressurizedHeight;
            dy = (yMax - yMin) / this->numPoints;
            dz = usInvert - dsInvert;
            isSteep = false; // in pressurized conduits, there is no such thing as a steep conduit
        }

        // We iterate until we've reached or exceeded the maximum depth.
        int count = 0;
        while (std::abs(yInit - yMax) > this->convergenceTol && yInit <= yMax)
        {
            this->errorCode = 0;

            double volume = 0.0;
            double yComp = 0.0;
		    double hf_reach = 0;

            // Now compute the point (downstream -> upstream if mild or
            // adverse, upstream -> downstream if steep).
            if (computeFreeOnly)
            {
                this->errorCode = ComputeFreeProfile(reach, flow, yInit, this->numSteps, isSteep, reverseSlope, this->g, this->kn, this->maxDepthFrac, yComp, volume, hf_reach);
            }
            else
            {
                this->errorCode = ComputeCombinedProfile(reach, flow, yInit, this->numSteps, isSteep, reverseSlope, this->g, this->kn, this->maxDepthFrac, yComp, volume, hf_reach);
            }

            // If the solution went imaginary, did not converge, or reached
            // the maximum pipe depth and not enough points were found, then
            // terminate early.  If there was another error (at_min_depth)
            // then continue to the next higher depth.
            if (this->errorCode)
            {
                if (this->errorCode == hpg::error::imaginary)
                    break;
                else if (this->errorCode == hpg::error::divergence)
                    break;
                else if (this->errorCode == hpg::error::at_max_depth && count < this->minCurvePoints)
                    break;
            }

            // If there was no error, then add the point to the curve.
            if (! this->errorCode)
            {
                if (isSteep)
                {
                    curve.push_back(hpg::point(yComp + dsInvert,
                                                 yInit + usInvert,
                                                 volume,
											     hf_reach));
                }
			    //else if (slope < 0.0)
                //{
			    //	curve.push_back(hpg::point(yComp + dsInvert,
			    //								 //yInit + dsInvert+(length*slope),
                //                                 //volume));
                //}

			    else if (reverseSlope)
                {
				    curve.push_back(hpg::point(yInit + usInvert,
											     yComp + dsInvert,
                                                 volume,
											     hf_reach));
                }

                else
                {
                    curve.push_back(hpg::point(yInit + dsInvert,
                                                 yComp + usInvert,
                                                 volume,
											     hf_reach));
                }

                count++;
            }

            // Go to the next depth.
            yInit = yInit + dy;
        }
    }
}


#if defined(_DEBUG)
#include <Windows.h>
#endif


// This is a wrapper around computeHpgCurve.  It returns true if a valid
// HPC was computed and false otherwise.  If false, then it also clears
// the curve variable so that no possibly bad values are stored.
bool HpgCreator::computeValidHpgCurve(const xs::Reach& reach, double flow, double pressurizedHeight, bool reverseSlope, double& yNormal, double& yCritical, hpg::hpgvec &curve)
{
    this->errorCode = 0;
    using namespace std;

    computeHpgCurve(reach, flow, pressurizedHeight, reverseSlope, yNormal, yCritical, curve);
    //OutputDebugStringA(std::to_string(flow).c_str());
    //OutputDebugStringA(" ");
    //OutputDebugStringA(std::to_string(curve.size()).c_str());
    //OutputDebugStringA("\n");
    //printf("Curve size: %d\n", curve.size());

    // We want to clear the error code if there was any, but we got valid points.
    if ((!this->errorCode ||
         this->errorCode == hpg::error::divergence ||
         this->errorCode == hpg::error::at_max_depth ||
         this->errorCode == hpg::error::imaginary ||
         this->errorCode == hpg::error::at_min_depth) &&
         curve.size() > this->minCurvePoints)
    {
        this->errorCode = 0;
        return true;
    }
    else
    {
        curve.clear();
        return false;
    }
}
