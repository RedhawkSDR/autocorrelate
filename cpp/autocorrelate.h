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
