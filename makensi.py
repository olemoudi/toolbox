#!/usr/bin/python

'''
Makensi module for common tasks

WebWalker class
	
	Creation
		WebWalker( 'name', maxworkers=1, responseCallback=callback)
	Methods
		webwalker.setDomain('www.google.es')
		webwalker.setPort(8080)		
		webwalker.GET('http://google.com/braubrau.html' [, {'User-Agent' : 'crappy browser'}])
		webwalker.POST('/braubrau.html' [,{'postparam1' : 'paramvalue1'} [, {'User-Agent' : 'crappy browser'}]])		
		webwalker.getHTTPResponseHeader('GET', 'www.google.es', '/search' [,{params}[,{headers}]], targetheader='Set-Cookie')
		webwalker.getHTTPResponseStatus('GET', 'www.google.es', '/search' [,{params}[,{headers}]])
		webwalker.getHTTPResponse('GET', 'www.google.es', '/search' [,{params}[,{headers}]])
		

ThreadPool class

	Creation
		pool = ThreadPool(maxworkers)
		
	Queuing tasks
		pool.queueTask(sortTask, (3, 30000), taskCallback)
		
		
		


'''


import urllib
import urllib2
import httplib
import threading
import time
import sys

class WebWalker():

	"Standard HTTP headers"
	WINHEADERS = {
	"User-Agent" : "Mozilla/5.0 (Windows; Windows NT 6.1; WOW64; rv:2.0b2) Gecko/20100720 Firefox/4.0b2",
	"Accept" : "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
	"Accept-Language" : "es-es,es;q=0.8,en-us;q=0.5,en;q=0.3",
	"Accept-Charset" : "ISO-8859-1,utf-8;q=0.7,*;q=0.7",
	}	
	

	def __init__(self, name='Web Walker', maxworkers=2, responseCallback=None):
		self.name=name
		self._domain=None		
		self._pool = ThreadPool(maxworkers)
		self._responseCallback = responseCallback
		self.setPort()	
		self.setProtocol()
	
	def setDomain(self, domain):
		self._domain = domain
	
	def setPort(self, port=80):
		self._port = port
	
	def setProtocol(self, protocol='http'):
		self._protocol = protocol
	
	def join(self):
		self._pool.joinAll()	
	
	def POST(self, url, params=None, headers=None):		
		if ( self._domain == None ) and ( url[0] == '/' ): 
			raise Exception('Wrong URL syntax: must set domain first')
			sys.exit(-1)
		else:
			if ( url[0] == '/'):
				url = '%s://%s:%i%s' %(self._protocol, self._domain, self._port, url) 
			elif ( url[:4] != 'http') and ( url[:5] != 'https') :				
				url = '%s://%s' % (self._protocol, url)	
		
			args = (url, params, headers)
			    # Insert tasks into the queue and let them run
			self._pool.queueTask(self._urlFunction, args, self._responseCallback)
			
	def GET(self, url, h=None):
		self.POST(url, params=None, headers=h)
			
	
	def getHTTPResponseHeader(self, method, domain, uri, params=None, headers=None, targetheader='Content-Type'):
		
		args = (method, domain, uri, params, headers, targetheader)
		self._pool.queueTask(self._gethttpRHFunction, args, self._responseCallback)
		

	def getHTTPResponseStatus(self, method, domain, uri, params=None, headers=None):
		
		args = (method, domain, uri, params, headers)
		self._pool.queueTask(self._gethttpStatusFunction, args, self._responseCallback)		
		
	def getHTTPResponse(self, method, domain, uri, params=None, headers=None):
		
		args = (method, domain, uri, params, headers)
		self._pool.queueTask(self._gethttpResponseFunction, args, self._responseCallback)				
			
	
	def _gethttpRHFunction(self, (method, domain, uri, p, h, targetheader)):	
	
		conn = httplib.HTTPConnection(domain)
		if ( p == None) and ( h == None ):
			conn.request( method, uri)
		elif ( p != None) and ( h == None ):
			conn.request(method, uri, urllib.urlencode(p))	
		elif ( p == None) and ( h != None ):
			conn.request( method, uri, headers=WebWalker.WINHEADERS.update(h))
		else:
			conn.request( method, uri, urllib.urlencode(p), WebWalker.WINHEADERS.update(h))
		
		response = conn.getresponse()
		th = response.getheader(targetheader)
		conn.close()
		
		return th
		
	def _gethttpStatusFunction(self, (method, domain, uri, p, h)):
		
		conn = httplib.HTTPConnection(domain)
		if ( p == None) and ( h == None ):
			conn.request( method, uri)
		elif ( p != None) and ( h == None ):
			conn.request(method, uri, urllib.urlencode(p))	
		elif ( p == None) and ( h != None ):
			conn.request( method, uri, headers=WebWalker.WINHEADERS.update(h))
		else:
			conn.request( method, uri, urllib.urlencode(p), WebWalker.WINHEADERS.update(h))
		
		response = conn.getresponse()
		conn.close()
		
		return response.status

	def _gethttpResponseFunction(self, (method, domain, uri, p, h)):
		
		conn = httplib.HTTPConnection(domain)
		if ( p == None) and ( h == None ):
			conn.request( method, uri)
		elif ( p != None) and ( h == None ):
			conn.request(method, uri, urllib.urlencode(p))	
		elif ( p == None) and ( h != None ):
			conn.request( method, uri, headers=WebWalker.WINHEADERS.update(h))
		else:
			conn.request( method, uri, urllib.urlencode(p), WebWalker.WINHEADERS.update(h))
		
		response = conn.getresponse()
		conn.close()
		
		return response		
	
	
	def _urlFunction(self, (url, p, h)):			
	
		if (p == None) and (h == None):
			_response = urllib2.urlopen(url)
		elif (p != None) and (h == None):
			_response = urllib2.urlopen(url, urllib.urlencode(p))
		elif (p == None) and (h != None):
			_response = urllib2.urlopen(url, headers=WebWalker.WINHEADERS.update(h))
		else:
			_response = urllib2.urlopen(url, urllib.urlencode(p), WebWalker.WINHEADERS.update(h))
				
		return _response.read()

		
	
			
class ThreadPool:

    """Flexible thread pool class.  Creates a pool of threads, then
    accepts tasks that will be dispatched to the next available
    thread."""
    
    def __init__(self, numThreads):

        """Initialize the thread pool with numThreads workers."""
        
        self.__threads = []
        self.__resizeLock = threading.Condition(threading.Lock())
        self.__taskLock = threading.Condition(threading.Lock())
        self.__tasks = []
        self.__isJoining = False
        self.setThreadCount(numThreads)

    def setThreadCount(self, newNumThreads):

        """ External method to set the current pool size.  Acquires
        the resizing lock, then calls the internal version to do real
        work."""
        
        # Can't change the thread count if we're shutting down the pool!
        if self.__isJoining:
            return False
        
        self.__resizeLock.acquire()
        try:
            self.__setThreadCountNolock(newNumThreads)
        finally:
            self.__resizeLock.release()
        return True

    def __setThreadCountNolock(self, newNumThreads):
        
        """Set the current pool size, spawning or terminating threads
        if necessary.  Internal use only; assumes the resizing lock is
        held."""
        
        # If we need to grow the pool, do so
        while newNumThreads > len(self.__threads):
            newThread = ThreadPoolThread(self)
            self.__threads.append(newThread)
            newThread.start()
        # If we need to shrink the pool, do so
        while newNumThreads < len(self.__threads):
            self.__threads[0].goAway()
            del self.__threads[0]

    def getThreadCount(self):

        """Return the number of threads in the pool."""
        
        self.__resizeLock.acquire()
        try:
            return len(self.__threads)
        finally:
            self.__resizeLock.release()

    def queueTask(self, task, args=None, taskCallback=None):

        """Insert a task into the queue.  task must be callable;
        args and taskCallback can be None."""
        
        if self.__isJoining == True:
            return False
        if not callable(task):
            return False
            
        while (len(self.__tasks) > (2*self.getThreadCount())):
            time.sleep(0.1)
        
        self.__taskLock.acquire()
        try:
            self.__tasks.append((task, args, taskCallback))
            return True
        finally:
            self.__taskLock.release()

    def getNextTask(self):

        """ Retrieve the next task from the task queue.  For use
        only by ThreadPoolThread objects contained in the pool."""
        
        self.__taskLock.acquire()
        try:
            if self.__tasks == []:
                return (None, None, None)
            else:
                return self.__tasks.pop(0)
        finally:
            self.__taskLock.release()
    
    def joinAll(self, waitForTasks = True, waitForThreads = True):

        """ Clear the task queue and terminate all pooled threads,
        optionally allowing the tasks and threads to finish."""
        
        # Mark the pool as joining to prevent any more task queueing
        self.__isJoining = True

        # Wait for tasks to finish
        if waitForTasks:
            while self.__tasks != []:
                time.sleep(.1)

        # Tell all the threads to quit
        self.__resizeLock.acquire()
        try:
            self.__setThreadCountNolock(0)
            self.__isJoining = True

            # Wait until all threads have exited
            if waitForThreads:
                for t in self.__threads:
                    t.join()
                    del t

            # Reset the pool for potential reuse
            self.__isJoining = False
        finally:
            self.__resizeLock.release()


        
class ThreadPoolThread(threading.Thread):

    """ Pooled thread class. """
    
    threadSleepTime = 0.1

    def __init__(self, pool):

        """ Initialize the thread and remember the pool. """
        
        threading.Thread.__init__(self)
        self.__pool = pool
        self.__isDying = False
        
    def run(self):

        """ Until told to quit, retrieve the next task and execute
        it, calling the callback if any.  """
        
        while self.__isDying == False:
            cmd, args, callback = self.__pool.getNextTask()
            # If there's nothing to do, just sleep a bit
            if cmd is None:
                time.sleep(ThreadPoolThread.threadSleepTime)
            elif callback is None:
                cmd(args)
            else:
                callback(cmd(args))
    
    def goAway(self):

        """ Exit the run loop next time through."""
        
        self.__isDying = True

import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders

def sendMail(fro, to, subject, text, files=[], server="localhost"):
        #print 'sending backup mail'
        assert type(to)==list
        assert type(files)==list
        msg = MIMEMultipart()
        msg['From'] = fro
        msg['To'] = COMMASPACE.join(to)
        msg['Date'] = formatdate(localtime=True)
        msg['Subject'] = subject

        msg.attach( MIMEText(text) )

        for file in files:
                part = MIMEBase('application', "octet-stream")
                part.set_payload( open(file,"rb").read() )
                Encoders.encode_base64(part)
                part.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(file))
                msg.attach(part)

        smtp = smtplib.SMTP(server)
        smtp.sendmail(fro, to, msg.as_string() )
        smtp.close()




# Usage example
if __name__ == "__main__":

    from random import randrange

    # Sample task 1: given a start and end value, shuffle integers,
    # then sort them
    
    def sortTask(data):
        print "SortTask starting for ", data
        numbers = range(data[0], data[1])
        for a in numbers:
            rnd = randrange(0, len(numbers) - 1)
            a, numbers[rnd] = numbers[rnd], a
        print "SortTask sorting for ", data
        numbers.sort()
        print "SortTask done for ", data
        return "Sorter ", data

    # Sample task 2: just sleep for a number of seconds.

    def waitTask(data):
        print "WaitTask starting for ", data
        print "WaitTask sleeping for %d seconds" % data
        time.sleep(data)
        return "Waiter", data

    # Both tasks use the same callback

    def taskCallback(data):
        print "Callback called for", data

    # Create a pool with three worker threads

    pool = ThreadPool(3)

    # Insert tasks into the queue and let them run
    pool.queueTask(sortTask, (1000, 100000), taskCallback)
    pool.queueTask(waitTask, 5, taskCallback)
    pool.queueTask(sortTask, (200, 200000), taskCallback)
    pool.queueTask(waitTask, 2, taskCallback)
    pool.queueTask(sortTask, (3, 30000), taskCallback)
    pool.queueTask(waitTask, 7, taskCallback)

    # When all tasks are finished, allow the threads to terminate
    pool.joinAll()			
	

