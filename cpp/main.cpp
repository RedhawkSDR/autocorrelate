#include <iostream>
#include "ossie/ossieSupport.h"

#include "autocorrelate.h"


int main(int argc, char* argv[])
{
    autocorrelate_i* autocorrelate_servant;
    Resource_impl::start_component(autocorrelate_servant, argc, argv);
    return 0;
}
