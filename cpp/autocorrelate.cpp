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
/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "autocorrelate.h"

PREPARE_LOGGING(autocorrelate_i)

AutocorrelatorProcessor::AutocorrelatorProcessor(std::vector<float>& outReal, std::vector<std::complex<float> >& outComplex, size_t correlationSz, long overlap, size_t numAverages, autocorrelator_output::type outType, bool zeroMean, bool zeroCenter):
	realOut(outReal),
	complexOut(outComplex),
	realAutocorrelator(outReal,correlationSz, overlap, numAverages, outType, zeroMean, zeroCenter),
	complexAutocorrelator(outComplex,correlationSz, overlap, numAverages, outType, zeroMean, zeroCenter),
	isComplex(false),
	output(outType),
	correlationSize(correlationSz),
	inputOverlap(overlap)
{
	//exponential averaging technique doesn't decimate output frames
	//we may change this later
	params.outputFramesPerInputFrame=1;
	setSubsize();
	setConsumeLen();
}

AutocorrelatorProcessor::SriParams AutocorrelatorProcessor::processReal(std::vector<float>& data)
{
	complexOut.clear();
	if (isComplex)
	{
		complexAutocorrelator.flush();
		isComplex=false;
	}
	realAutocorrelator.run(data);
	if (params.forcePush)
	{
		SriParams ret = params;
		params.forcePush=false;
		return ret;
	}
	return params;
}
AutocorrelatorProcessor::SriParams AutocorrelatorProcessor::processComplex(std::vector<std::complex<float> >&data)
{
	realOut.clear();
	if (!isComplex)
	{
		realAutocorrelator.flush();
		isComplex=true;
	}
	complexAutocorrelator.run(data);
	if (params.forcePush)
	{
		SriParams ret = params;
		params.forcePush=false;
		return ret;
	}
	return params;
}

void AutocorrelatorProcessor::setCorrelationSize(const size_t& newVal)
{
	realAutocorrelator.setCorrelationSize(newVal);
	complexAutocorrelator.setCorrelationSize(newVal);
	correlationSize=newVal;
	setSubsize();
	setConsumeLen();
}
void AutocorrelatorProcessor::setOverlap(const size_t& newVal)
{
	realAutocorrelator.setOverlap(newVal);
	complexAutocorrelator.setOverlap(newVal);
	inputOverlap=newVal;
	setConsumeLen();
}
void AutocorrelatorProcessor::setNumAverages(const size_t& newVal)
{
	realAutocorrelator.setNumAverages(newVal);
	complexAutocorrelator.setNumAverages(newVal);
}
void AutocorrelatorProcessor::setOutputType(autocorrelator_output::type& newVal)
{
	realAutocorrelator.setOutputType(newVal);
	complexAutocorrelator.setOutputType(newVal);
	output=newVal;
	setSubsize();
}
void AutocorrelatorProcessor::setZeroMean(const bool& newVal)
{
	realAutocorrelator.setZeroMean(newVal);
	complexAutocorrelator.setZeroMean(newVal);
}
void AutocorrelatorProcessor::setZeroCenter(const bool& newVal)
{
	realAutocorrelator.setZeroCenter(newVal);
	complexAutocorrelator.setZeroCenter(newVal);
}
void AutocorrelatorProcessor::setSubsize()
{
	size_t subsize;
	if (output==autocorrelator_output::SUPERIMPOSED)
		subsize = correlationSize;
	else
		subsize =2*correlationSize-1;
	if (subsize!= params.subsize)
	{
		params.subsize = subsize;
		params.forcePush=true;
	}

}
void AutocorrelatorProcessor::setConsumeLen()
{
	size_t consumeLen= correlationSize-inputOverlap;
	if (consumeLen!= params.consumeLen)
	{
		params.consumeLen = consumeLen;
		params.forcePush=true;
	}
}

autocorrelate_i::autocorrelate_i(const char *uuid, const char *label) :
    autocorrelate_base(uuid, label)
{
		setPropertyChangeListener("correlationSize", this, &autocorrelate_i::correlationSizeChanged);
		setPropertyChangeListener("inputOverlap", this, &autocorrelate_i::inputOverlapChanged);
		setPropertyChangeListener("numAverages", this, &autocorrelate_i::numAveragesChanged);
		setPropertyChangeListener("outputType", this, &autocorrelate_i::outputTypeChanged);
		setPropertyChangeListener("zeroMean", this, &autocorrelate_i::zeroMeanChanged);
		setPropertyChangeListener("zeroCenter", this, &autocorrelate_i::zeroCenterChanged);
}

autocorrelate_i::~autocorrelate_i()
{
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		delete i->second;
}


autocorrelator_output::type autocorrelate_i::translateOutputType() {
	autocorrelator_output::type outType;
      if (outputType == "ROTATED")
              outType = autocorrelator_output::ROTATED;
      else if (outputType == "SUPERIMPOSED")
              outType = autocorrelator_output::SUPERIMPOSED;
      else
      {
              if (outputType != "NORMAL")
            	  LOG_WARN(autocorrelate_i, "You have chosen an invalid outputType "<< outputType<<". Using NORMAL instead");
              outType = autocorrelator_output::STANDARD;
      }
      return outType;
}


/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

	Time:
	    To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (autocorrelate_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        A callback method can be associated with a property so that the method is
        called each time the property value changes.  This is done by calling 
        setPropertyChangeListener(<property name>, this, &autocorrelate_i::<callback method>)
        in the constructor.
            
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to autocorrelate.cpp
        autocorrelate_i::autocorrelate_i(const char *uuid, const char *label) :
            autocorrelate_base(uuid, label)
        {
            setPropertyChangeListener("scaleValue", this, &autocorrelate_i::scaleChanged);
        }

        void autocorrelate_i::scaleChanged(const std::string& id){
            std::cout << "scaleChanged scaleValue " << scaleValue << std::endl;
        }
            
        //Add to autocorrelate.h
        void scaleChanged(const std::string&);
        
        
************************************************************************************************/
int autocorrelate_i::serviceFunction()
{
	bulkio::InFloatPort::dataTransfer *tmp = dataFloat_in->getPacket(bulkio::Const::BLOCKING);
	if (not tmp) { // No data is available
		return NOOP;
	}

	// NOTE: You must make at least one valid pushSRI call
	bool pushSRI = tmp->sriChanged;
	AutocorrelatorProcessor::SriParams sriParams;
	{
		boost::mutex::scoped_lock lock(processorLock);
		map_type::iterator i = processors.find(tmp->streamID);
		if (i==processors.end())
		{
			LOG_DEBUG(autocorrelate_i, "Creating new processor for stream: '" << tmp->streamID << "'");
			pushSRI = true;
			autocorrelator_output::type outputType = translateOutputType();
			AutocorrelatorProcessor* ap = new  AutocorrelatorProcessor(realOutput, complexOutput, correlationSize, inputOverlap, numAverages, outputType,zeroMean, zeroCenter);
			map_type::value_type processor(tmp->streamID, ap);
			i = processors.insert(processors.end(),processor);
		}
		if (tmp->SRI.mode==0) //real input
			sriParams = i->second->processReal(tmp->dataBuffer);
		else  //complex input
		{
			std::vector<std::complex<float> >* cxInput =(std::vector<std::complex<float> >*)(&tmp->dataBuffer);
			LOG_DEBUG(autocorrelate_i, "Processing cx data: '" << tmp->streamID << "' "<< cxInput->size());
			sriParams = i->second->processComplex(*cxInput);
		}

		if (tmp->EOS) {
			delete i->second;
			processors.erase(i);
			LOG_DEBUG(autocorrelate_i, "Received EOS for stream: '" << tmp->streamID << "'");
		}
	}
	if (pushSRI || sriParams.forcePush)
	{
		LOG_DEBUG(autocorrelate_i, "pushing SRI: '" << tmp->streamID << "' ");
		tmp->SRI.subsize=sriParams.subsize;
		tmp->SRI.ydelta=sriParams.consumeLen*tmp->SRI.xdelta*sriParams.outputFramesPerInputFrame;
		dataFloat_out->pushSRI(tmp->SRI);
	}
	if (!realOutput.empty())
	{
		LOG_DEBUG(autocorrelate_i, "sending real output: '" << tmp->streamID << "' "<< realOutput.size());
		dataFloat_out->pushPacket(realOutput, tmp->T, tmp->EOS, tmp->streamID);
	}
	if (!complexOutput.empty())
	{
		LOG_DEBUG(autocorrelate_i, "sending complex output: '" << tmp->streamID << "' "<< complexOutput.size());
		std::vector<float>* outVec = (std::vector<float>*)(&complexOutput);
		dataFloat_out->pushPacket(*outVec, tmp->T, tmp->EOS, tmp->streamID);
	}

	delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
	return NORMAL;

}

void autocorrelate_i::correlationSizeChanged(const std::string&)
{
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		i->second->setCorrelationSize(correlationSize);
}
void autocorrelate_i::inputOverlapChanged(const std::string&)
{
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		i->second->setOverlap(inputOverlap);
}
void autocorrelate_i::numAveragesChanged(const std::string&)
{
	//zero numAverages is not valid - change to 1
	if (numAverages==0)
		numAverages=1;
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		i->second->setNumAverages(numAverages);
}
void autocorrelate_i::outputTypeChanged(const std::string&)
{
	autocorrelator_output::type outputType = translateOutputType();
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		i->second->setOutputType(outputType);
}
void autocorrelate_i::zeroMeanChanged(const std::string&)
{
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		i->second->setZeroMean(zeroMean);
}
void autocorrelate_i::zeroCenterChanged(const std::string&)
{
	boost::mutex::scoped_lock lock(processorLock);
	for (map_type::iterator i = processors.begin(); i!=processors.end(); i++)
		i->second->setZeroMean(zeroCenter);
}
