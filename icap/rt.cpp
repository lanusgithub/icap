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


#include "icap.h"


void ICAP::SetCurrentNodeInflow(const std::string& nodeId, var_type flow)
{
    m_geometry->setRealTimeInputFlow(nodeId, flow);
}


void ICAP::SetCurrentNodeHead(const std::string& nodeId, var_type head)
{
    m_realTimeDsHead = true;
    m_geometry->setRealTimeInputHead(nodeId, head);
}


var_type ICAP::GetCurrentNodeHead(const std::string& nodeId)
{
    return m_geometry->getRealTimeNodeHead(nodeId);
}


var_type ICAP::GetCurrentNodeInflow(const std::string& nodeId)
{
    return m_geometry->getRealTimeNodeInflow(nodeId);
}


void ICAP::AddSource(const std::string& nodeId)
{
    m_geometry->addRealTimeInput(nodeId);
}


void ICAP::SetFlowFactor(var_type flowFactor)
//
//  Input:   flow factor
//  Output:  none
//  Purpose: modifies scale factor on inflows
//
{
    for (geometry::Geometry::NodeIter iter = m_geometry->beginNode(); iter != m_geometry->endNode(); iter++)
    {
        iter->second->setInflowFactor(flowFactor);
    }
}


void ICAP::EnableRealTimeStatus()
{
    m_realTimeFlows = true;
    if (m_geometry != NULL)
    {
        m_geometry->enableRealTimeStatus();
    }
}
