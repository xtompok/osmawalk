#!/usr/bin/env python2
import urllib2  # handling url 
import json     # json data format
import random
import time
import os

alphabet = "0ABCD2EFGH4IJKLMN6OPQRST8UVWXYZ-1abcd3efgh5ijklmn7opqrst9uvwxyz."

def coordsToString(arr):
    ox = 0
    oy = 0
    result = ""

    for lat, lon in arr:
        lat = float(lat)
        lon = float(lon)
        #coords = arr[i].toWGS84()
        x = round((lon + 180) * (1 << 28) / 360)
        y = round((lat + 90) * (1 << 28) / 180)

        dx = x - ox
        dy = y - oy
        #/*if (!dx and !dy) { continue }*/

        result += serializeNumber(dx, x)
        result += serializeNumber(dy, y)

        ox = x
        oy = y

    return result

 
def serializeNumber(delta, orig):
    code = ""
    delta = int(delta)
    orig  = int(orig)
    if (delta >= -1024 and delta < 1024):
        code += alphabet[((delta + 1024) >> 6)]
        code += alphabet[((delta + 1024) & 63)]

    elif (delta >= -32768 and delta < 32768):
        value = 0x20000 | (delta + 32768)
        code += alphabet[((value >> 12) & 63)]
        code += alphabet[((value >>  6) & 63)]
        code += alphabet[(value & 63)]

    else:
        value = 0x30000000 | (orig & 0xFFFFFFF)
        code += alphabet[((value >> 24) & 63)]
        code += alphabet[((value >> 18) & 63)]
        code += alphabet[((value >> 12) & 63)]
        code += alphabet[((value >>  6) & 63)]
        code += alphabet[(value & 63)]

    return code

    
def readValueFromData(atributes, data):
    # searches for atribute value in data 
    result = []
    for atribute in atributes:
        tmp = data
        if tmp.find( atribute ) == -1:
            print "Could not find atribute %s. " % atribute
            return (None, None)
        else:
            # slice it! find atribute, add len of atribute and add 2 for '="' e.g. 'length="'
            tmp = tmp[ tmp.find( atribute ) + len( atribute ) + 2 : ]
            # find '"' which stands for the end of value
            tmp = tmp[ : tmp.find('"') ]
            result.append( int( tmp ) )

    return result


class FetchTransfer(object):

    def __init__(self, filename='transfers'):

        self.filename = filename + '.json'
        self.time     = time.time()

        # try to locate filename
        if os.path.isfile( self.filename ):
            with open( self.filename , 'r' ) as f: # default name transfers
                self.transfers = json.load(f)
                print "Successfully loaded %s. " % self.filename
        else: 
            print "Can not locate %s file..." % self.filename
            self.transfers = {}        


    def fetchTransfer(self, from_, to): 
        """
        'from_' and 'to' are tuples (latitude, longitude)
    
        For given 'from_' and 'to' fetches path distance 
        and estimated time from_ seznam-api
        distance in meters, time in seconds
        If error occurs, (None, None) is returned
        """
        # order coordinates by latitude
        if from_[0] > to[0]:
            tmp     = from_
            from_   = to
            to      = tmp
    
        # convert from_ and to into one string 'from_ to'
        coords = str(from_) + ' ' + str(to)
    
        # check if the coords are already in dict, if not load them from api
        if coords not in self.transfers:
            fetch_time = time.time()          # get current time
            
            if self.time - fetch_time < 0.3:  # check for wait
                t_r = random.random() + 0.2
                print "We are too fast for Seznam api ({0:.2f}s). Sleeping for {1:.2f}s".format(self.time - fetch_time, t_r)
                time.sleep(t_r)
    
            # open url with
            print "Connecting to seznam-api and fetching data..."
            data      = urllib2.urlopen("http://api4.mapy.cz/route?geometry=" + coordsToString([from_, to]) + "&criterion=turist1")
            r_data    = data.read()    
            self.time = time.time()

            # if there is literally an error in r_data
            if r_data.find('error') > 0:
                print "An error has appeared in the XML data for %s .." % coords
                length, time_s = ( None, None )
                return None
            else:
                # read length and time from r_data
                length, time_s = readValueFromData( atributes=['length', 'time'], data=r_data )
    
            # add coords data to self.transfers
            self.transfers[ coords ] = { 'length'  : length,
                                         'time'    : time_s }

            return r_data
    
        else:
            # get length and time from transfers
            length   = self.transfers[ coords ][ 'length' ]
            time_s   = self.transfers[ coords ][ 'time' ]

            return r_data
    

    def dumpData(self):

        # save self.transfers in self.filename as json data
        with open( self.filename, 'w') as f:
            json.dump( self.transfers, f, sort_keys=True, indent=4)
