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

#ifndef AUTOCORRELATE_IMPL_H
#define AUTOCORRELATE_IMPL_H

#include "autocorrelate_base.h"
#include "DataTypes.h"
#include "Autocorrelate.h"

class autocorrelate_i;

class AutocorrelatorProcessor
{
public:
	struct SriParams
	{
		size_t subsize;
		size_t consumeLen;
		size_t outputFramesPerInputFrame;
		bool forcePush;
	};
	AutocorrelatorProcessor(std::vector<float>& outReal, std::vector<std::complex<float> >& outComplex, size_t correlationSz, long overlap, size_t numAverages, autocorrelator_output::type outType, bool zeroMean, bool zeroCenter);

	SriParams processReal(std::vector<float>& data);
	SriParams processComplex(std::vector<std::complex<float> >&data);

	void setCorrelationSize(const size_t& newVal);
	void setOverlap(const size_t& newVal);
	void setNumAverages(const size_t& newVal);
	void setOutputType(autocorrelator_output::type& newVal);
	void setZeroMean(const bool& newVal);
	void setZeroCenter(const bool& newVal);

private:
	void setSubsize();
	void setConsumeLen();

	std::vector<float>& realOut;
	std::vector<std::complex<float> >& complexOut;

	Autocorrelator<float> realAutocorrelator;
	Autocorrelator<std::complex<float> > complexAutocorrelator;
	bool isComplex;
	autocorrelator_output::type output;
	size_t correlationSize;
	long inputOverlap;
	SriParams params;
};

class autocorrelate_i : public autocorrelate_base
{
    ENABLE_LOGGING
    public:
        autocorrelate_i(const char *uuid, const char *label);
        ~autocorrelate_i();
        int serviceFunction();

    private:
        typedef std::map<std::string, AutocorrelatorProcessor*> map_type;
        void correlationSizeChanged(const std::string&);
        void inputOverlapChanged(const std::string&);
        void numAveragesChanged(const std::string&);
        void outputTypeChanged(const std::string&);
        void zeroMeanChanged(const std::string&);
        void zeroCenterChanged(const std::string&);
        autocorrelator_output::type translateOutputType();
        RealVector realOutput;
        ComplexVector complexOutput;

        map_type processors;
        boost::mutex processorLock;
};

#endif
