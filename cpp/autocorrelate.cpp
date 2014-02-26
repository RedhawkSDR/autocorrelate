/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "autocorrelate.h"

PREPARE_LOGGING(autocorrelate_i)

autocorrelate_i::autocorrelate_i(const char *uuid, const char *label) :
    autocorrelate_base(uuid, label),
        autocorrelator(realOutput, correlationSize, inputOverlap, numAverages,translateOutputType(),zeroMean, zeroCenter),
        paramsChanged(false),
        updateCorrelationSize(false),
        updateInputOverlap(false),
        updateNumAverages(false),
        updateOutputType(false),
        updateZeroMean(false),
        updateZeroCenter(false)

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
}


Autocorrelator::OUTPUT_TYPE autocorrelate_i::translateOutputType() {
      Autocorrelator::OUTPUT_TYPE outType;
      if (outputType == "ROTATED")
              outType = Autocorrelator::ROTATED;
      else if (outputType == "SUPERIMPOSED")
              outType = Autocorrelator::SUPERIMPOSED;
      else
      {
              if (outputType != "NORMAL")
                      std::cout<<"You have chosen an invalid outputType "<< outputType<<". Using NORMAL instead"<<std::endl;
              outType = Autocorrelator::STANDARD;
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
	if (tmp->SRI.mode==1)
	{
		std::cout<<"complex autocorrelation currently not supported"<<std::endl;
		delete tmp;
		return NORMAL;
	}

	//use one big paramsChanged to reduce the number of booleans we will evaluate per loop
	if (paramsChanged)
	{
		std::cout<<"paramsChanged"<<std::endl;
		if (updateCorrelationSize)
		{
			std::cout<<"component settign correlationSize to "<<correlationSize<<std::endl;
			autocorrelator.setCorrelationSize(correlationSize);
			pushSRI=true;
			updateCorrelationSize=false;
		}
		if (updateInputOverlap)
		{
			autocorrelator.setOverlap(inputOverlap);
			pushSRI=true;
			updateInputOverlap=false;
		}
		if (updateNumAverages)
		{
			std::cout<<"setNumAverages "<<numAverages<<std::endl;
			autocorrelator.setNumAverages(numAverages);
			std::cout<<"setNumAverages done"<<std::endl;
			pushSRI=true;
			updateNumAverages=false;
		}
		if (updateOutputType)
		{
			Autocorrelator::OUTPUT_TYPE outType = translateOutputType();
			std::cout<<"updateOutputType "<<outputType<<", "<< outType<<std::endl;
			autocorrelator.setOutputType(outType);
			updateOutputType=false;
		}
		if (updateZeroMean)
		{
			autocorrelator.setZeroMean(zeroMean);
			updateZeroMean=false;
		}
		if (updateZeroCenter)
		{
			std::cout<<"calling updateZeroCenter with "<<zeroCenter<<std::endl;
			autocorrelator.setZeroCenter(zeroCenter);
			updateZeroCenter=false;
		}
		paramsChanged=false;
		std::cout<<"paramsChanged done "<<zeroCenter<<std::endl;
	}
	autocorrelator.run(tmp->dataBuffer);
	if (pushSRI)
	{
		size_t outFrameSize = 2*correlationSize-1;
		if (outputType=="SUPERIMPOSED")
			outFrameSize = correlationSize;

		tmp->SRI.subsize=outFrameSize;//32*1024;//2*outputSize-1;
		tmp->SRI.ydelta=(correlationSize-inputOverlap)*tmp->SRI.xdelta*numAverages;
		dataFloat_out->pushSRI(tmp->SRI);
	}
	if (!realOutput.empty())
		dataFloat_out->pushPacket(realOutput, tmp->T, tmp->EOS, tmp->streamID);

	delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
	return NORMAL;

}

void autocorrelate_i::correlationSizeChanged(const std::string&)
{
	updateCorrelationSize=true;
	paramsChanged=true;
}
void autocorrelate_i::inputOverlapChanged(const std::string&)
{
	updateInputOverlap=true;
	paramsChanged=true;
}
void autocorrelate_i::numAveragesChanged(const std::string&)
{
	//zero numAverages is not valid - change to 1
	if (numAverages==0)
		numAverages=1;
	updateNumAverages=true;
	paramsChanged=true;
}
void autocorrelate_i::outputTypeChanged(const std::string&)
{
	std::cout<<"outputTypeChanged "<<std::endl;
	updateOutputType=true;
	paramsChanged=true;
}
void autocorrelate_i::zeroMeanChanged(const std::string&)
{
	updateZeroMean=true;
	paramsChanged=true;
}
void autocorrelate_i::zeroCenterChanged(const std::string&)
{
	updateZeroCenter=true;
	paramsChanged=true;
}
