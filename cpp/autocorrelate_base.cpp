/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components autocorrelate.
 *
 * REDHAWK Basic Components autocorrelate is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components autocorrelate is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */
#include "autocorrelate_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

autocorrelate_base::autocorrelate_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    ThreadedComponent()
{
    loadProperties();

    dataFloat_in = new bulkio::InFloatPort("dataFloat_in");
    addPort("dataFloat_in", dataFloat_in);
    dataFloat_out = new bulkio::OutFloatPort("dataFloat_out");
    addPort("dataFloat_out", dataFloat_out);
}

autocorrelate_base::~autocorrelate_base()
{
    delete dataFloat_in;
    dataFloat_in = 0;
    delete dataFloat_out;
    dataFloat_out = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void autocorrelate_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Resource_impl::start();
    ThreadedComponent::startThread();
}

void autocorrelate_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Resource_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void autocorrelate_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Resource_impl::releaseObject();
}

void autocorrelate_base::loadProperties()
{
    addProperty(correlationSize,
                4096,
                "correlationSize",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(inputOverlap,
                0,
                "inputOverlap",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(numAverages,
                1,
                "numAverages",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(outputType,
                "NORMAL",
                "outputType",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(zeroMean,
                true,
                "zeroMean",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(zeroCenter,
                false,
                "zeroCenter",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}


