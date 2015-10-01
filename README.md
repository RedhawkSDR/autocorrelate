# REDHAWK Basic Components rh.autocorrelate
 
## Description

Contains the source and build script for the REDHAWK Basic Components rh.autocorrelate. This component is a Frequency domain implemenation of a windowed autocorrelation algorithim.  This algorthim works by windowing the input data to break it up into separate frames.  Each frame is independently autocorrelated with each other using a &quot;full&quot; autocorrelation which includes the full transient response.  This is efficiently computed in the frequency domain.


## Installation Instructions
This asset requires the rh.dsp and rh.fftlib shared libraries. These must be installed in order to build and run this asset.
To build from source, run the `build.sh` script found at the top level directory. To install to $SDRROOT run, `build.sh install`

## Copyrights

This work is protected by Copyright. Please refer to the [Copyright File](COPYRIGHT) for updated copyright information.

## License

REDHAWK Basic Components rh.autocorrelate is licensed under the GNU General Public License (GPL).