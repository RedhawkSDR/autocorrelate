#!/usr/bin/env python
import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils import sb
import math
import time

import matplotlib.pyplot
import numpy
import scipy.fftpack
import random
import struct

from omniORB import any
from ossie.cf import CF
from omniORB import CORBA

def plotFreqResponse(signal, fftNum, sampleRate):
    #freqResponse = [20*math.log(max(abs(x),1e-9),10) for x in scipy.fftpack.fftshift(scipy.fftpack.fft(signal,fftNum))]
    freqResponse = scipy.fftpack.fftshift(scipy.fftpack.fft(signal,fftNum))
    freqAxis =  scipy.fftpack.fftshift(scipy.fftpack.fftfreq(fftNum,1.0/sampleRate))
    matplotlib.pyplot.plot(freqAxis, freqResponse)
    matplotlib.pyplot.show()
    
#     mean = sum(signal)/float(len(signal))
#     print "mean is %s" %mean 
#     meanRemvoed=  [x-mean for x in signal]
#     #freqResponseA = [20*math.log(max(abs(x),1e-9),10) for x in scipy.fftpack.fftshift(scipy.fftpack.fft(meanRemvoed,fftNum))]
#     freqResponseA = scipy.fftpack.fftshift(scipy.fftpack.fft(meanRemvoed,fftNum))
#     matplotlib.pyplot.plot(freqAxis, freqResponseA)
#     matplotlib.pyplot.show()
# 
#     matplotlib.pyplot.plot([x-y for x, y in zip(freqResponse,freqResponseA)]) 
#     matplotlib.pyplot.show()

        

def toClipboard(data):
    import pygtk
    pygtk.require('2.0')
    import gtk
    
    # get the clipboard
    clipboard = gtk.clipboard_get()
    txt = str(data)
    clipboard.set_text(txt)
    
    # make our data available to other applications
    clipboard.store()

def genSinWave(fs, freq, numPts, cx=True, startTime=0, amp=1):
    xd = 1.0/fs
    phase =  2*math.pi*startTime
    phaseInc = 2*math.pi*freq/fs
    output = []
    for i in xrange(numPts): 
        output.append(amp*math.cos(phase))
        if cx:
            output.append(amp*math.sin(phase))
        phase+=phaseInc
    return output

def genSqWave(fs, freq, numPts, cx=True, startTime=0, amp=1):
    xd = 1.0/fs
    output = []
    freqDelta = 1.0/freq
    nextTransition = 0  
    for i in xrange(numPts): 
        output.append(amp)
        startTime+=xd
        if startTime > nextTransition:
            amp*=-1
            nextTransition+=freqDelta
    return output


class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in autocorrelate"""

    def setUp(self):
        """Set up the unit test - this is run before every method that starts with test
        """
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        self.src = sb.DataSource()
        self.sink = sb.DataSink()
        
        #setup my components
        self.setupComponent()
        
        self.comp.start()
        self.src.start()
        self.sink.start()
        
        #do the connections
        self.src.connect(self.comp)        
        self.comp.connect(self.sink)

    
    def tearDown(self):
        """Finish the unit test - this is run after every method that starts with test
        """
        self.comp.stop()
        #######################################################################
        # Simulate regular component shutdown
        self.comp.releaseObject()
        self.sink.stop()      
        ossie.utils.testing.ScaComponentTestCase.tearDown(self)
    
    def testScaBasicBehavior(self):
        pass
    
    def testRealData(self):
        sampleRate = 10e3
        #input = [x/sampleRate for x in range(int(5e4))]
        #input = genSinWave(sampleRate, 100,int(5e4),False)
        #input = genSqWave(sampleRate, 100,int(5e4),False)
        #input = [random.gauss(0,1.0) for _ in xrange(int(5e4))]
        #input = [random.random() for _ in xrange(int(5e4))]
        #input = genSinWave(sampleRate, 100,int(5e4),False)
        #input = [x+1 for x in input]
        
        f = file('/home/bsg/bitsout','r')
        s = f.read()
        shortInput = struct.unpack('%sh'%(len(s)/2),s)
        input = [float(x) for x in shortInput]
        
        numFrames = 3800
        print "numFrames = ", numFrames
#         for i in xrange(1,numFrames):
#             iSlice = input[i*360:(i+1)*360]
#             out = [x+y for x, y in zip(out,iSlice)]
#         matplotlib.pyplot.plot(range(360), out)
#         matplotlib.pyplot.show()
#         return 
        input = input[:numFrames*360]


        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=3000
        self.comp.inputOverlap=0
        self.comp.numAverages=3000
        print self.comp.outputType
        #self.comp.outputType="ROTATED"
        self.comp.outputType="SUPERIMPOSED"
        #self.comp.outputType="NORMAL"
        print self.comp.outputType
                
        inSlice = input[:int(self.comp.correlationSize)]
        
        output = self.main(input, False, sampleRate)
        print "got output", len(output)
        print "len input", len(input)
        print self.comp.correlationSize
        
        #toClipboard(input)
        c = numpy.correlate(inSlice,inSlice,'full')
        print len(c)
        
        outFrame = output[-1]
        
        #plotFreqResponse(outFrame, 1024,1)
        
        if output:
        
            frameSize = len(outFrame)
            #matplotlib.pyplot.plot([1.0/sampleRate*x for x in range(frameSize)], input[:frameSize])
            #matplotlib.pyplot.show()
            matplotlib.pyplot.plot(range(frameSize), outFrame)
            #matplotlib.pyplot.show()
            #matplotlib.pyplot.plot([1.0/sampleRate*x for x in range(frameSize)], output[1])
            
            #matplotlib.pyplot.plot(range(frameSize), c)
            matplotlib.pyplot.show()
            
            print "start"
            print c[:10]
            print outFrame[:10]
            
            print "stop"
            
            print c[-5:]
            print outFrame[-5:]
            
            maxDif = max([abs(x-y) for x, y in zip(c,outFrame)])
            print "maxDif = ", maxDif
            assert(maxDif<.01)
    
    def setupComponent(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp, None)
        self.assertEqual(self.comp.ref._non_existent(), False)
        self.assertEqual(self.comp.ref._is_a("IDL:CF/Resource:1.0"), True)
        
        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)
        
        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)
            
        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
    
    
    def main(self,inData, complexData = True, sampleRate = 1.0, streamID    = "teststream"):
        """The main engine for all the test cases - configure the equation, push data, and get output
           As applicable
        """
        #data processing is asynchronos - so wait until the data is all processed
        count=0
        self.src.push(inData,
                      complexData = complexData,
                      sampleRate=sampleRate)
        out=[]
        while True:
            newOut = self.sink.getData()
            if newOut:
                out.extend(newOut)
                count=0
            elif count==100:
                break
            time.sleep(.01)
            count+=1
        sri= self.sink.sri()
        framed = []
        i=0
        if sri.subsize>0:
            while i*sri.subsize<len(out):
                framed.append(out[i*sri.subsize:(i+1)*sri.subsize])
                i+=1
            out=framed
        return out
        
    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.bulkio.bulkio_helpers,
    #   ossie.utils.bluefile.bluefile_helpers
    # for modules that will assist with testing components with BULKIO ports
    
if __name__ == "__main__":
    ossie.utils.testing.main("../autocorrelate.spd.xml") # By default tests all implementations
