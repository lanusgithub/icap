// ==============================================================================
// ICAP License
// ==============================================================================
// University of Illinois/NCSA
// Open Source License
// 
// Copyright (c) 2014-2016 University of Illinois at Urbana-Champaign.
// All rights reserved.
// 
// Developed by:
// 
//     Nils Oberg
//     Blake J. Landry, PhD
//     Arthur R. Schmidt, PhD
//     Ven Te Chow Hydrosystems Lab
// 
//     University of Illinois at Urbana-Champaign
// 
//     https://vtchl.illinois.edu
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// 
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
// 
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
// 
//     * Neither the names of the Ven Te Chow Hydrosystems Lab, University of
// 	  Illinois at Urbana-Champaign, nor the names of its contributors may be
// 	  used to endorse or promote products derived from this Software without
// 	  specific prior written permission.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.


#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "../model/units.h"
#include "../util/parse.h"

#include "icap_geometry.h"
#include "rt_inflow.h"
#include "ucf.h"

using namespace geometry;
namespace fs = boost::filesystem;


units::Units* UCS = new units::Units(units::UnitSystem::Units_US);
ucf::Ucf* UCF = new CfsUnitsConversion();


void IcapGeometry::enableRealTimeStatus()
{
    for (auto iter = beginNode(); iter != endNode(); iter++)
    {
        iter->second->clearInflowObjects();
    }
}

void IcapGeometry::addRealTimeInput(std::string nodeId)
{
    boost::algorithm::to_lower(nodeId);
    std::shared_ptr<Node> node = getNode(nodeId);
    if (node == NULL)
    {
        return;
    }

    std::shared_ptr<RealTimeInflow> inflow = std::shared_ptr<RealTimeInflow>(new RealTimeInflow());

    node->clearInflowObjects();
    node->attachInflow(inflow);

    this->rtInflowMap.insert(std::pair<std::string, std::shared_ptr<RealTimeInflow>>(nodeId, inflow));
}

void IcapGeometry::setRealTimeInputFlow(std::string nodeId, var_type flow)
{
    boost::algorithm::to_lower(nodeId);
    if (this->rtInflowMap.count(nodeId) == 0)
    {
        return;
    }

    this->rtInflowMap[nodeId]->setCurrentInflow(flow);
}

void IcapGeometry::setRealTimeInputHead(std::string nodeId, var_type head)
{
    boost::algorithm::to_lower(nodeId);
    std::shared_ptr<Node> node = getNode(nodeId);
    if (node == NULL)
    {
        return;
    }
    
    node->variable(variables::NodeDepth) = head - node->getInvert();
}

var_type IcapGeometry::getRealTimeNodeHead(std::string nodeId)
{
    boost::algorithm::to_lower(nodeId);
    std::shared_ptr<Node> node = getNode(nodeId);
    if (node == NULL)
    {
        return ERROR_VAL;
    }

    return node->variable(variables::NodeDepth) + node->getInvert();
}

var_type IcapGeometry::getRealTimeNodeInflow(std::string nodeId)
{
    boost::algorithm::to_lower(nodeId);
    std::shared_ptr<Node> node = getNode(nodeId);
    if (node == NULL)
    {
        return ERROR_VAL;
    }

    return node->variable(variables::NodeFlow);
}


bool IcapGeometry::processOptions()
{
    this->freeSurfaceOnlyComputations = false;
    this->routeStep = 1;

    std::vector<std::string> options = getOptionNames();

    std::vector<std::string> reqs;
    
    reqs.push_back("start_date");
    reqs.push_back("start_time");
    reqs.push_back("end_date");
    reqs.push_back("end_time");
    reqs.push_back("flow_units");
    reqs.push_back("hpg_path");
    reqs.push_back("routing_step");
    reqs.push_back("report_step");

    for (auto req : reqs)
    {
        if (!hasOption(req))
        {
            setErrorMessage("Unable to find option " + req);
            return false;
        }
    }

    if (getOption("flow_units") != "cfs")
    {
        setErrorMessage("Invalid option value for flow_units");
        return false;
    }
    else
    {
        if (UCF != NULL)
            delete UCF;
        UCF = new CfsUnitsConversion();
    }

    if (!DateTime::tryParseDate(getOption("start_date"), this->startDate, Format::M_D_Y))
    {
        setErrorMessage("Unable to parse option start_date");
        return false;
    }

    if (!DateTime::tryParseTime(getOption("start_time"), this->startTime))
    {
        setErrorMessage("Unable to parse option start_time");
        return false;
    }
        
    if (!DateTime::tryParseDate(getOption("end_date"), this->endDate, Format::M_D_Y))
    {
        setErrorMessage("Unable to parse option end_date");
        return false;
    }

    if (!DateTime::tryParseTime(getOption("end_time"), this->endTime))
    {
        setErrorMessage("Unable to parse option end_time");
        return false;
    }

    this->hpgPath = getOption("hpg_path");
    if (!boost::filesystem::exists(this->hpgPath))
    {
        fs::path parentDir = fs::path(this->geomFilePath).parent_path();
        this->hpgPath = (parentDir / this->hpgPath).string();
        if (!boost::filesystem::exists(this->hpgPath))
        {
            setErrorMessage("Invalid path to HPG folder '" + this->hpgPath + "'");
            return false;
        }
    }

    if (!tryParse(getOption("routing_step"), this->routeStep))
    {
        setErrorMessage("Non-numeric invalid routing_step option is provided.");
        return false;
    }

    TimeSpan dt;
    if (!DateTime::tryParseTime(getOption("report_step"), dt))
    {
        setErrorMessage("Non-numeric invalid report_step option is provided.");
        return false;
    }
    this->reportStep = dt.getTotalSeconds();

    if (hasOption("free_surface_only"))
    {
        std::string opt(getOption("free_surface_only"));
        boost::algorithm::to_lower(opt);
        if (opt == "true")
        {
            this->freeSurfaceOnlyComputations = true;
        }
    }

    return true;
}

void IcapGeometry::startTimestep(const DateTime& dateTime)
{
    this->currentDateTime = dateTime;
    for (auto iter = Geometry::beginNode(); iter != Geometry::endNode(); iter++)
    {
        iter->second->startInflow(dateTime);
    }
}

var_type IcapGeometry::getNodeVariable(id_type nodeIdx, variables::Variables var)
{
    return getNode(nodeIdx)->variable(var);
}

var_type IcapGeometry::getLinkVariable(id_type linkIdx, variables::Variables var)
{
    return getLink(linkIdx)->variable(var);
}

void IcapGeometry::setNodeVariable(id_type nodeIdx, variables::Variables var, var_type value)
{
    getNode(nodeIdx)->variable(var) = value;
}

void IcapGeometry::setLinkVariable(id_type linkIdx, variables::Variables var, var_type value)
{
    getLink(linkIdx)->variable(var) = value;
}

void IcapGeometry::updateNodeStatistic(id_type nodeId, statvariables::StatVariables var, var_type value)
{
    //TODO
}

/// <summary>
/// This function updates the given system statistic by adding the given value.
/// </summary>
void IcapGeometry::updateSystemStatistic(statvariables::StatVariables var, var_type value)
{
    //TODO:
}

DateTime IcapGeometry::getCurrentDateTime()
{
    //TODO:
    return DateTime();
}

void IcapGeometry::resetDepths()
{
    //TODO:
}

void IcapGeometry::resetTimestep()
{
    for (auto iter = Geometry::beginNode(); iter != Geometry::endNode(); iter++)
    {
        iter->second->resetFlow();
        iter->second->resetDepth();
    }

    for (auto iter = Geometry::beginLink(); iter != Geometry::endLink(); iter++)
    {
        iter->second->resetFlow();
        iter->second->resetDepth();
    }
}
