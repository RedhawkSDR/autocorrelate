#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
# source distribution.
# 
# This file is part of REDHAWK Basic Components autocorrelate.
# 
# REDHAWK Basic Components autocorrelate is free software: you can redistribute it and/or modify it under the terms of 
# the GNU General Public License as published by the Free Software Foundation, either 
# version 3 of the License, or (at your option) any later version.
# 
# REDHAWK Basic Components autocorrelate is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with this 
# program.  If not, see http://www.gnu.org/licenses/.

import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils import sb
import math
import time

DISPLAY = False
if DISPLAY:
    import matplotlib.pyplot

import numpy
import scipy.fftpack
import random
import struct

from omniORB import any
from ossie.cf import CF
from omniORB import CORBA
import __builtin__

#Numpy changed the api for correlate in version 1.5
from distutils.version import LooseVersion
NUMPY_GREATER_15 = LooseVersion(numpy.__version__) >= LooseVersion('1.5')
_orig_correlate = numpy.correlate
def correlatewrap(data,filter,command, *kwargs):
    return _orig_correlate(data,numpy.conj(filter),command, *kwargs)
if NUMPY_GREATER_15:
    numpy.correlate = correlatewrap


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

def genCorrelatedData(numPts, taps, mean=0, stddev=1,cx=False):
    output=[]
    input =[]
    #simple FIR filter to correlate the data
    
    for i in xrange(numPts):
        if cx:
            next = complex(random.gauss(mean,stddev),random.gauss(mean,stddev)) #white noise input data
        else:
            next = random.gauss(mean,stddev) #white noise input data
        input.append(next)
        for index, value in taps:
            if index<=i:
                next+=value*output[-index]
        output.append(next)
    return output        

def unpackCx(data):
    out =[]
    for val in data:
        out.append(val.real)
        out.append(val.imag)
    return out

def packCx(data):
    real=None
    out=[]
    for val in data:
        if real==None:
            real=val
        else:
            out.append(complex(real,val))
            real=None
    return out            

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
    
    def testReal1(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest()
    
    def testReal2(self):        
        self.comp.zeroCenter=True;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest()

    def testReal3(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest() 

    def testReal4(self):
        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest() 
    
    def testReal5(self):   
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest()

    def testReal6(self):        
        self.comp.zeroCenter=True;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest()

    def testReal7(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest() 

    def testReal8(self):
        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest() 

    def testReal9(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest()
    
    def testReal10(self):        
        self.comp.zeroCenter=True;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest()

    def testReal11(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest() 

    def testReal12(self):
        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest() 

    def testCx1(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest(True)
    
    def testCx2(self):        
        self.comp.zeroCenter=True;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest(True)

    def testCx3(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest(True) 

    def testCx4(self):
        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.datatest(True) 
    
    def testCx5(self):   
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest(True)

    def testCx6(self):        
        self.comp.zeroCenter=True;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest(True)

    def testCx7(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest(True) 

    def testCx8(self):
        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "ROTATED"
        self.datatest(True) 

    def testCx9(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest(True)
    
    def testCx10(self):        
        self.comp.zeroCenter=True;
        self.comp.zeroMean=False;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest(True)

    def testCx11(self):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest(True) 

    def testCx12(self):
        self.comp.zeroCenter=True;
        self.comp.zeroMean=True;
        self.comp.correlationSize=101
        self.comp.inputOverlap=0
        self.comp.numAverages=0
        self.comp.outputType = "SUPERIMPOSED"
        self.datatest(True) 

    def testEosReal(self):
        self.eosTest(False)

    def testEosCx(self):
        self.eosTest(True)
    
    def testMultiStreamReal(self):
        self.multiStreamTest(False)

    def testMultiStreamCx(self):
        self.multiStreamTest(True)
        
    def eosTest(self, cx=False):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=50
        self.comp.inputOverlap=10
        self.comp.numAverages=0
        
        sampleRate=10e3
        streamID = 'testStream'
        outFrameA, mainInput = self.datatest(cx, sampleRate,streamID=streamID)
        output = self.main(mainInput, cx, sampleRate,eos=True, streamID=streamID)
        outFrameB = output[0]
        assert len(outFrameB)==len(outFrameA)
        self.assertTrue(__builtin__.any([abs(x-y)>.1 for x, y in zip(outFrameA, outFrameB)]))
        
        #work arround a bug here to force an SRI push by the src
        self.src._sri=None
        output = self.main(mainInput, cx, sampleRate, streamID=streamID)
        outFrameC = output[0]
        assert len(outFrameC)==len(outFrameA)
        self.assertTrue(all([abs(x-y)<.1 for x, y in zip(outFrameC, outFrameA)]))
        
    def multiStreamTest(self, cx=False):
        self.comp.zeroCenter=False;
        self.comp.zeroMean=False;
        self.comp.correlationSize=50
        self.comp.inputOverlap=10
        self.comp.numAverages=0
        
        sampleRate=10e3
        outFrameA, mainInput = self.datatest(cx, sampleRate, streamID='stream_a')
        output = self.main(mainInput, cx, sampleRate,streamID = 'stream_a')
        outFrameB = output[0]
        assert len(outFrameB)==len(outFrameA)
        self.assertTrue(__builtin__.any([abs(x-y)>.1 for x, y in zip(outFrameA, outFrameB)]))
        
        output = self.main(mainInput, cx, sampleRate, streamID = 'stream_b')
        outFrameC = output[0]
        assert len(outFrameC)==len(outFrameA)
        self.assertTrue(all([abs(x-y)<.1 for x, y in zip(outFrameC, outFrameA)]))
    
    def datatest(self,cx=False, sampleRate = 10e3,input=None, streamID="test_stream"):  
        #create random data but correlate it by running it through a one-tap IIR filter
        if input==None:
            tapIndex = 17        
            taps=((tapIndex,.98),)
            input = genCorrelatedData(self.comp.correlationSize,taps,0, 1, cx)
        else:
            tapIndex=None

        if cx:
            mainInput = unpackCx(input)
        else:
            mainInput = input
        #now run it through the component
        output = self.main(mainInput, cx, sampleRate, streamID)
        
        #now calculate the "expected output"
        if self.comp.zeroMean:
            #if zero mean - remove the mean from our correlation Index
            avg = sum(input)/len(input)
            corlInput= [x-avg for x in input]
        else:
            corlInput= input
        
        if cx:
            corlConj = [x.conjugate() for x in corlInput]
            c = numpy.correlate(corlInput,corlConj,'full')
        else:
            c = numpy.correlate(corlInput,corlInput,'full')
        
        frameLen = 2*self.comp.correlationSize-1
        if self.comp.outputType=="NORMAL":
            #centerIndex at the middle and tap indicies on either side 
            centerIndex = self.comp.correlationSize-1
        else:
            #centerIndex at the center
            centerIndex = 0
        if tapIndex!=None:
            if self.comp.outputType=="NORMAL":
                #centerIndex at the middle and tap indicies on either side 
                maxIndicies = [centerIndex-tapIndex, centerIndex+tapIndex]
            else:
                #grab the second half of the data
                secondHalf = c[:self.comp.correlationSize-1]
                #this is the first half of the data
                c = list(c[self.comp.correlationSize-1:])
                if self.comp.outputType=="SUPERIMPOSED":
                    #adjust the frameLen to be the correlation size
                    frameLen =  int(self.comp.correlationSize)
                    #add the flipped second half to the firsthalf to superimpose the data
                    i = frameLen-1
                    for val in secondHalf:
                        c[i]+=val
                        i-=1            
                    #only one maxIndex that we care about since we've added the two max indicies on top of each other
                    maxIndicies = [tapIndex]
                else: #ROTATED
                    #extend the second half
                    c.extend(secondHalf)
                    #maxIndicies are near the beginning and end of the frame
                    maxIndicies = [tapIndex, frameLen-tapIndex]
        
        #if we need to zero the CenterIndex - then do so
        if self.comp.zeroCenter:
            c[centerIndex]=0
        
        #the outFrame is the first frame we care about
        outFrame = output[0]
        if DISPLAY:
            matplotlib.pyplot.plot(range(frameLen), outFrame)
            matplotlib.pyplot.plot(range(frameLen), c)
            matplotlib.pyplot.show()
        
        #make sure the outputFrame is the right lenght
        self.assertTrue(len(outFrame)==len(c)==frameLen)
        #make sure our output is the same as the numpy output
        self.assertTrue(all([abs(x-y)<.1 for x, y in zip(outFrame, c)]))
        if tapIndex !=None:
            #sort the output to find where the biggest indicies are
            if cx:
                maxVals = [(y.real,x) for (x,y) in enumerate(outFrame)]
            else:
                maxVals = [(y,x) for (x,y) in enumerate(outFrame)]
            
            maxVals.sort(reverse=True)
            
            if self.comp.zeroCenter:
                #we've already zeroed out the centerIndex - so check the start of the maxValues to get the maxIndicies
                calMaxIndicies = [x[1] for x in maxVals[:len(maxIndicies)]]
            else:
                if self.comp.outputType!="SUPERIMPOSED":
                    #typical case
                    #make sure the cetnerIndex is the maxIndex
                    self.assertTrue(maxVals[0][1]==centerIndex)
                    #our next highest indicies are associated with the correlation we've introduced
                    calMaxIndicies = [x[1] for x in maxVals[1:3]]
                else:
                    if maxIndicies[0]==centerIndex:
                        #use the second index
                        calMaxIndicies=[maxIndicies[1]]
                    else:
                        #this is a weird corner case -- the superposition actually makes the correlation output bigger at the tapIndex then at the centerIndex!
                        calMaxIndicies=[maxIndicies[0]]
                
            calMaxIndicies.sort()
            self.assertTrue(maxIndicies==calMaxIndicies)
        return outFrame, mainInput
    
    def setupComponent(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        execparams["DEBUG_LEVEL"] = 4
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
    
    
    def main(self,inData, complexData = True, sampleRate = 1.0, streamID    = "teststream", eos=False):
        """The main engine for all the test cases - configure the equation, push data, and get output
           As applicable
        """
        #data processing is asynchronos - so wait until the data is all processed
        #print len(inData), complexData, sampleRate, streamID, eos
        count=0
        self.src.push(inData,
                      streamID=streamID,
                      complexData = complexData,
                      sampleRate=sampleRate,
                      EOS=eos)
        out=[]
        while True:
            newOut = self.sink.getData()
            if newOut:
                if complexData:
                    newOut = packCx(newOut)
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
            #break out the data into frames according to the subsize
            while i*sri.subsize<len(out):
                framed.append(out[i*sri.subsize:(i+1)*sri.subsize])
                i+=1
            out=framed
        return out
    
if __name__ == "__main__":
    ossie.utils.testing.main("../autocorrelate.spd.xml") # By default tests all implementations
