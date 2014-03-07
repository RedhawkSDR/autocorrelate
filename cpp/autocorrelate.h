/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components autocorrelate.
 *
 * REDHAWK Basic Components autocorrelate is free software: you can redistribute it and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components autocorrelate is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef AUTOCORRELATE_IMPL_H
#define AUTOCORRELATE_IMPL_H

#include "autocorrelate_base.h"
#include "DataTypes.h"
#include "Autocorrelate.h"

class autocorrelate_i;

class autocorrelate_i : public autocorrelate_base
{
    ENABLE_LOGGING
    public:
        autocorrelate_i(const char *uuid, const char *label);
        ~autocorrelate_i();
        int serviceFunction();

    private:
        void correlationSizeChanged(const std::string&);
        void inputOverlapChanged(const std::string&);
        void numAveragesChanged(const std::string&);
        void outputTypeChanged(const std::string&);
        void zeroMeanChanged(const std::string&);
        void zeroCenterChanged(const std::string&);
        Autocorrelator::OUTPUT_TYPE translateOutputType();

        RealVector realOutput;
        Autocorrelator autocorrelator;
        bool paramsChanged;
        bool updateCorrelationSize;
        bool updateInputOverlap;
        bool updateNumAverages;
        bool updateOutputType;
        bool updateZeroMean;
        bool updateZeroCenter;
};

#endif
