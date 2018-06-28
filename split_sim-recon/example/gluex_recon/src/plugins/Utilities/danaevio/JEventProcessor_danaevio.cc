// JEventProcessor_danaevio.cc
//
//
// JANA event processor plugin writes out evio DOM tree to file OR sends
//    events to receiver (e.g. event display) via TCP socket
//
//
// Implements JANA command-line parameters:
//
//    EVIO:FILENAME     output file name, default "dana_events.evio"
//                      use "socket" (lower case) to communicate with event display
//    EVIO:HOST         specify host for socket communications, default is "localhost"
//    EVIO:PORT         specify port for socket communications, default is 3309 (0xCED)
//    EVIO:BUFSIZE      serialized event internal buffer size, default 200000 words
//    EVIO:SOCKETTRY    number of times to try connecting to the socket
//    EVIO:SOCKETWAIT   number of seconds between tries
//
//
//  dana_evio_dict.xml is corresponding evio2xml dictionary
//
//  E.g. to run:
//     $GLUEX_RECON_HOME/bin/Linux_CentOS5-x86_64-gcc4.1.2/hd_ana --plugin=danaevio -PEVIO:DANAEVIO="all" ../Event.hddm
//
//
//  Elliott Wolin, 19-Jul-2010


#include <danaevio/DDANAEVIODOMTree.h>
#include <danaevio/DDANAEVIO_factory.h>
#include <danaevio/JEventProcessor_danaevio.h>
#include <danaevio/dana_evio_dict.h>



// evio output file name, use EVIO:FILENAME command-line parameter to override
static string evioFileName = "dana_events.evio";


// for evio file output
static evioFileChannel *chan;


// evio TCP host and port for socket-based output
static bool evioIOAbort          = false;        // true if unrecoverable IO error
static string evioHost           = "localhost";
static int evioPort              = 0xCED;        // 3309
static FILE* evioFILE            = NULL;
static int evioSocket            = 0;  
static uint32_t socketHeader[3]  = {0xCEBAF,1,0};
static uint32_t *socketBuffer;
static int evioSocketTry         = 6;
static int evioSocketWait        = 5;


// internal evio buffer size, use EVIO:BUFSIZE command-line parameter to override
static int evioBufSize=750000;


// mutex for serializing writing to file
static pthread_mutex_t evioMutex = PTHREAD_MUTEX_INITIALIZER;


// from Dave Heddle
static FILE *initSocket(const char *ipAddress, int port, int *sock);


// from JApplication
extern jana::JApplication *japp;



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


JEventProcessor_danaevio::JEventProcessor_danaevio() {
    

  jout << endl << "  Default JEventProcessor_danaevio invoked" << endl << endl;


  // check for EVIO:FILENAME output file name parameter
  // if "socket" then output using TCP socket
  gPARMS->SetDefaultParameter("EVIO:FILENAME",evioFileName);
  jout << endl << "  EVIO output file name is " << evioFileName << endl << endl;


  // check for socket parameters
  gPARMS->SetDefaultParameter("EVIO:HOST",evioHost);
  gPARMS->SetDefaultParameter("EVIO:PORT",evioPort);
  gPARMS->SetDefaultParameter("EVIO:SOCKETTRY",evioSocketTry);
  gPARMS->SetDefaultParameter("EVIO:SOCKETWAIT",evioSocketWait);


  // check for EVIO:BUFSIZE internal buffer size parameter
  gPARMS->SetDefaultParameter("EVIO:BUFSIZE",evioBufSize);
  jout << endl << "  EVIO internal buf size is " << evioBufSize << endl << endl;
  if(evioFileName=="socket") {
    jout << endl << "  EVIO TCP socket host is " << evioHost<< endl << endl;
    jout << endl << "  EVIO TCP socket port is " << evioPort<< endl << endl;
    jout << endl << "  EVIO TCP socket try is  " << evioSocketTry << endl << endl;
    jout << endl << "  EVIO TCP socket wait is " << evioSocketWait << endl << endl;
  }
  
  
  // open file channel or TCP socket
  if(evioFileName!="socket") {

    // file I/O
    try {
      chan = new evioFileChannel(evioFileName,"w",evioBufSize);
      chan->open();
      
    } catch (evioException e) {
      jerr << endl << "  ?evioException in JEventProcessor_danaevio" << endl << endl 
           << e.toString() << endl;
      japp->Quit();
      evioIOAbort=true;

    } catch (...) {
      jerr << endl << "  ?unknown exception in JEventProcessor_danaevio, unable to open output file" << endl << endl;
      japp->Quit();
      evioIOAbort=true;
    }


  } else {

    // TCP socket I/O
    // allocate buffer to hold serialized event
    socketBuffer = new uint32_t[evioBufSize];

    // open socket
    evioFILE = initSocket(evioHost.c_str(),evioPort,&evioSocket);
    if(evioFILE==NULL) {
      jerr << endl << " ?JEventProcessor_danaevio...unable to open socket" << endl << endl;
      japp->Quit();
      evioIOAbort=true;
      return;
    }
  }

}


//----------------------------------------------------------------------------


JEventProcessor_danaevio::~JEventProcessor_danaevio() {
    
  if(evioIOAbort)return;


  // close file or socket
  if(evioFileName!="socket") {

    // file I/O
    try {
      chan->close();
      delete(chan);
      
    } catch (evioException e) {
      jerr << endl << "  ?evioException in ~JEventProcessor_danaevio" << endl << endl 
           << e.toString() << endl;
    } catch (...) {
      jerr << endl << "  ?unknown exception in ~JEventProcessor_danaevio, unable to close output file" << endl << endl;
    }

  } else {
    
    // TCP socket I/O
    if(evioFILE!=NULL) {
      fflush(evioFILE);
      fclose(evioFILE);
      delete(socketBuffer);
    }
  }

}


//----------------------------------------------------------------------------


jerror_t JEventProcessor_danaevio::brun(JEventLoop *eventLoop, int32_t runnumber) {

  static bool first_time = true;
  unsigned int n;


  // has file or socket open failed?
  if(evioIOAbort)return(UNRECOVERABLE_ERROR);


  // get write lock
  pthread_mutex_lock(&evioMutex);

  
  // create dictionary banks from DDANAEVIO factory tagMap<string, pair<uint16_t,uint8_t> >
  //  and write out as first event in file
  if(first_time) {
    first_time=false;
    
    try {
      evioDOMTree tree(1,0);
      evioDOMNodeP name =  evioDOMNode::createEvioDOMNode<string>   (1,1);
      evioDOMNodeP tag  =  evioDOMNode::createEvioDOMNode<uint16_t> (1,2);
      evioDOMNodeP num  =  evioDOMNode::createEvioDOMNode<uint8_t>  (1,3);
      tree << name << tag << num;
      
      const map< string, pair<uint16_t,uint8_t> > *theMap = DDANAEVIO_factory::getTagMapPointer();
      map< string, pair<uint16_t,uint8_t> >::const_iterator iter;
      for(iter=theMap->begin(); iter!=theMap->end(); iter++) {
        *name << iter->first;
        *tag  << iter->second.first;
        *num  << iter->second.second;
      }    
      
      // file or socket I/O
      if(evioFileName!="socket") {
        chan->write(tree);

      } else {
        tree.toEVIOBuffer(socketBuffer,evioBufSize);
        socketHeader[2]=4*(socketBuffer[0]+1);
        n  = fwrite(socketHeader,sizeof(uint32_t),3,evioFILE);
        n += fwrite(socketBuffer,sizeof(uint32_t),socketBuffer[0]+1,evioFILE);
        if(n!=(3+socketBuffer[0]+1)) {
          jerr << " ?JEventProcessor_danaevio::brun...unable to write to socket" << endl;
          return(UNRECOVERABLE_ERROR);
        }
        fflush(evioFILE);
      }



    } catch (evioException e) {
      jerr << endl << "  ?evioException in ~JEventProcessor_danaevio::brun, unable to write to file" << endl << endl 
           << e.toString() << endl;

    } catch (...) {
      jerr << endl << "  ?unknown exception in ~JEventProcessor_danaevio::brun" << endl << endl;
    }
  }
  
  // unlock
  pthread_mutex_unlock(&evioMutex);
  

  return(NOERROR);
}


//----------------------------------------------------------------------------


jerror_t JEventProcessor_danaevio::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
    
  unsigned int n;


  // has file or socket open failed?
  if(evioIOAbort)return(UNRECOVERABLE_ERROR);


  // get evio trees
  vector<const DDANAEVIODOMTree*> evioTrees; 
  eventLoop->Get(evioTrees);
  if(evioTrees.size()<=0)return(NOERROR);


  // get write lock
  pthread_mutex_lock(&evioMutex);


  // write out all evio trees
  if(evioFileName!="socket") {
    
    // file I/O
    for(unsigned int i=0; i<evioTrees.size(); i++) {
      try {
        chan->write(evioTrees[i]->tree);
        
      } catch (evioException e) {
        jerr << endl << "  ?evioException in JEventProcessor_danaevio::evnt" << endl << endl 
             << e.toString() << endl;
      } catch (...) {
        jerr << endl << "  ?unknown exception in JEventProcessor_danaevio::evnt, unable to write to file" << endl << endl;
      }
    }

  } else {

    // socket I/o
    for(unsigned int i=0; i<evioTrees.size(); i++) {
      try {

        evioTrees[i]->tree.toEVIOBuffer(socketBuffer,evioBufSize);
        socketHeader[2]=4*(socketBuffer[0]+1);
        n  = fwrite(socketHeader,sizeof(uint32_t),3,evioFILE);
        n += fwrite(socketBuffer,sizeof(uint32_t),socketBuffer[0]+1,evioFILE);
        if(n!=(3+socketBuffer[0]+1)) {
          jerr << " ?JEventProcessor_danaevio::evnt...unable to write to socket" << endl;
          return(UNRECOVERABLE_ERROR);
        }
        fflush(evioFILE);

      } catch (...) {
        jerr << endl << "  ?unknown exception in JEventProcessor_danaevio::evnt, unable to write to socket " << endl << endl;
      }
    }
  }
    

  // unlock
  pthread_mutex_unlock(&evioMutex);

  
  // done
  return NOERROR;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


// from Dave Heddle's note on CEDSocket
//  ejw, 23-Jul-2010


//return a stream that wraps the socket for writing, or
//NULL if it fails for any reason. Upon return the reference
//for the socket will be in the variable sock
FILE *initSocket(const char *ipAddress, int port, int *sock) {

  // Create a stream socket using TCP and IPv4
  *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (*sock < 0) {
    jerr << endl;
    jerr << " ?initSocket...socket() failed";
    jerr << endl;
    return NULL;
  }


  // Construct the server address structure
  struct sockaddr_in servAddr;             // Server address
  memset(&servAddr, 0, sizeof(servAddr));  // Zero out structure
  servAddr.sin_family = AF_INET;           // IPv4 address family
	

  // get host entry using ascii host name
  struct hostent *myHostEnt = gethostbyname(ipAddress);
  if(myHostEnt==NULL) {
    jerr << endl;
    jerr << " ?initSocket...unable to gethostbyname()";
    jerr << endl;
    return NULL;
  }


  // Convert address to 4-byte form using ascii dotted-decimal form
  struct in_addr **myList = (in_addr **)myHostEnt->h_addr_list;
  int rtnVal = inet_pton(AF_INET, inet_ntoa(*myList[0]), &servAddr.sin_addr.s_addr);
  if (rtnVal == 0) {
    jerr << endl;
    jerr << " ?initSocket...inet_pton() failed. Invalid address string";
    jerr << endl;
    return NULL;
  } 
  else if (rtnVal < 0) {
    jerr << endl;
    jerr << " ?initSocket...inet_pton() failed";
    jerr << endl;
    return NULL;
  }
  servAddr.sin_port = htons(port); // Server port


  // try a number of times to establish the connection to the server
  int i=0;
  while (true) {
    i++;
    if(connect(*sock, (struct sockaddr *) &servAddr, sizeof(servAddr))>=0) {
      jout << "initSocket...connection successful on attempt " << i << endl;
      break;
      
    } else if (i<evioSocketTry) {
      jerr << "   ...initSocket connection attempt " << i << " failed, trying again..." << endl;
      sleep(evioSocketWait);
      continue;
      
    } else {
      jerr << endl;
      jerr << " ?initSocket...connect() failed after " << evioSocketTry << " attempts" << endl;
      jerr << endl;
      return NULL;
    }
  }


  //wrap the socket in an output stream
  return fdopen(*sock, "w");
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
