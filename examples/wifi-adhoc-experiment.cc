/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *   Description 
 */

// Standard C++ modules
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iterator>
#include <iostream>
#include <map>
#include <string>
#include <sys/time.h>
#include <vector>
#include <ctime> // time_t
#include <cstdio>

// ns3 modules
#include "ns3/core-module.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/internet-apps-module.h"
#include "ns3/uniform-helper.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WifiAdhocExperiment");

/* SensorInforTransferApp from lln-home.cc -------------------------------------------------------*/
class SensorInfoTransferApp : public Application
{
public:

  static TypeId
  GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::SensorInfoTransferApp")
          .SetParent<Application> ()
      .SetGroupName("Applications")
          .AddConstructor<SensorInfoTransferApp> ()
          .AddAttribute ("Frequency", "Frequency of interest packets",
             StringValue ("1.0"),
             MakeDoubleAccessor (&SensorInfoTransferApp::m_frequency),
             MakeDoubleChecker<double> ())
      .AddAttribute ("Remote", "The address of the destination",
         AddressValue (),
         MakeAddressAccessor (&SensorInfoTransferApp::m_peer),
         MakeAddressChecker ())
      .AddAttribute ("Protocol", "The type of protocol to use.",
         TypeIdValue (UdpSocketFactory::GetTypeId ()),
         MakeTypeIdAccessor (&SensorInfoTransferApp::m_tid),
         MakeTypeIdChecker ())
      .AddAttribute ("PacketNumber", "The number of packets to send.",
         StringValue ("10"),
         MakeIntegerAccessor (&SensorInfoTransferApp::m_pktCountLimit),
         MakeIntegerChecker<uint32_t> ())
             ;
          return tid;
  }

  SensorInfoTransferApp ()
  : m_socket (0)
  , m_peer (Address ())
  , m_pktSize (256)
  , m_firstTime (true)
  , m_frequency (1.0)
  , m_tid (UdpSocketFactory::GetTypeId ())
  , m_pktCountLimit (10)  // initialize the number of packets to send
  , m_pktCount (0)        // Current packet count (number of packets already sent)
  {
  }

  Ptr<Socket>
  GetSocket () const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_socket;
  }

  void
  ScheduleNextPacket ()
  {
    NS_LOG_FUNCTION (this << "m_firstTime = " << m_firstTime);
    
    if (m_firstTime)
      {
      // In order to not make everything send at 1.0, we push back the initial start
      // by a random between 0 and 100 milliseconds.
      Ptr<UniformRandomVariable> uniRandom  = CreateObject<UniformRandomVariable> ();

      double start = uniRandom->GetValue(0.0, 100.0);

      NS_LOG_FUNCTION ("SensorApp: First packet out from" << this 
        << "for " << Address(m_peer) 
        << "in = " << start << " sec.");

      m_sendEvent = Simulator::Schedule (MilliSeconds (start),
             &SensorInfoTransferApp::SendPacket, this);
      m_firstTime = false;
      NS_LOG_FUNCTION ("After Setting the schedule event."); // --
      }
    else if (!m_sendEvent.IsRunning ())
      {
        if (m_pktCount < m_pktCountLimit)
        {
          NS_LOG_FUNCTION ("SensorApp: Next packet out from" << this 
            << "for " << m_peer
            << "in = " << Seconds (1.0 / m_frequency) << " sec.");
          m_sendEvent = Simulator::Schedule (Seconds (1.0 / m_frequency),
            &SensorInfoTransferApp::SendPacket, this);
        }
      }
  }

  void
  SendPacket ()
  {
    NS_LOG_FUNCTION (this << " Send Packet called!");
    // Create the socket
    m_socket = Socket::CreateSocket (GetNode (), m_tid);

    // Bind to the socket
    if (Inet6SocketAddress::IsMatchingType (m_peer))
      {
        m_socket->Bind6 ();
      }
    else if (InetSocketAddress::IsMatchingType (m_peer) ||
    PacketSocketAddress::IsMatchingType (m_peer))
      {
        m_socket->Bind ();
      }

    // Set the relevant settings
    m_socket->Connect (m_peer);
    m_socket->SetAllowBroadcast (true);

    // Create the packet
    Ptr<Packet> packet = Create<Packet> (m_pktSize);
    //Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> ("hello"), 5);

    NS_LOG_FUNCTION ("Pointer to the new packet = " << packet << " of size = "<< packet->GetSize());

    // Send the packet
    m_socket->Send (packet);

    // Count the packet just sent
    m_pktCount++; 

    // Close the socket
    if (m_socket != 0)
      {
        m_socket->Close ();
      }

    // Schedule the next transmission
    ScheduleNextPacket ();
  }

protected:
  void
  StartApplication ()
  {
    NS_LOG_FUNCTION ("+++ Call to Start Application ");
    NS_LOG_FUNCTION_NOARGS ();
    ScheduleNextPacket ();
  }

private:
  Ptr<Socket> m_socket;
  Address     m_peer;
  uint32_t    m_pktSize;
  bool        m_firstTime;
  double      m_frequency;
  EventId     m_sendEvent;
  TypeId      m_tid;
  uint32_t    m_pktCountLimit; // Limit for the number of packets to send
  uint32_t    m_pktCount; // Current packet count (number of packets already sent)
};

class SensorInfoTransferAppHelper
{
public:

  SensorInfoTransferAppHelper (std::string protocol, Address address)
  {
    m_factory.SetTypeId ("ns3::SensorInfoTransferApp");
    m_factory.Set ("Protocol", StringValue (protocol));
    m_factory.Set ("Remote", AddressValue (address));
  }

  void
  SetAttribute (std::string name, const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }

  ApplicationContainer
  Install (NodeContainer c) const
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
      {
        apps.Add (InstallPriv (*i));
      }

    return apps;
  }

  ApplicationContainer
  Install (Ptr<Node> node) const
  {
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer
  Install (std::string nodeName) const
  {
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv (node));
  }

private:
  Ptr<Application>
  InstallPriv (Ptr<Node> node) const
  {
    Ptr<Application> app = m_factory.Create<Application> ();
    node->AddApplication (app);

    return app;
  }

  ObjectFactory m_factory;
};

NS_OBJECT_ENSURE_REGISTERED (SensorInfoTransferApp);
/*------------------------------------------------------------------------------------------------*/

class WifiAdhocExperiment
{
public:
  /// Init experiment
  WifiAdhocExperiment ();
  /**
   * Configure the experiment from command line arguments
   *
   * \param argc command line argument count
   * \param argv command line arguments
   */
  void Configure (int argc, char ** argv);
  /**
   * Run the experiment
   * \returns the experiment status
   */
  void Run ();
private:
  time_t  	m_begin;
  time_t  	m_end;
  char 	  	m_experimentDir[250]; 
  char  		m_currentExecDir[250]; 
  char 		  m_posFile[250];
  char 		  m_flowFile[250];
  uint16_t  m_n;
  bool      m_verbose;
  string    m_phyMode;
  double    m_rss;  // -dBm
  double    m_totalTime;
  ApplicationContainer m_flows[100]; // statically set a max of 100 flows
  uint8_t   m_flowIndex;
  uint32_t  m_numPackets;
  
  /// List of network nodes
  NodeContainer nodes;
  /// List of all devices
  NetDeviceContainer devices;
  /// Addresses of interfaces:
  Ipv6InterfaceContainer interfaces;
  
private:
  /// Create nodes and setup their mobility
  void CreateNodes ();
  /// Install internet m_stack on nodes
  void InstallInternetStack ();
  /// Install applications
  void InstallApplication ();
  /// Print devices diagnostics
  void Report ();
};

WifiAdhocExperiment::WifiAdhocExperiment () :
  m_begin (0),
  m_end (0),
  m_experimentDir ("/home/alioua/Projects/Tests/experiments"),
  m_verbose (false),
  m_phyMode ("DsssRate1Mbps"),
  m_rss (-80),
  m_totalTime (250),
  m_flowIndex (0),
  m_numPackets(10)
{
}

void
WifiAdhocExperiment::Configure (int argc, char *argv[])
{
	/*
  CommandLine cmd;
  cmd.AddValue ("name", "Number of nodes in a row grid", m_xSize);
  cmd.AddValue ("y-size", "Number of rows in a grid", m_ySize);
  cmd.AddValue ("step",   "Size of edge in our grid (meters)", m_step);
  // Avoid starting all mesh nodes at the same time (beacons may collide)
  cmd.AddValue ("start",  "Maximum random start delay for beacon jitter (sec)", m_randomStart);
  cmd.AddValue ("time",  "Simulation time (sec)", m_totalTime);
  cmd.AddValue ("packet-interval",  "Interval between packets in UDP ping (sec)", m_packetInterval);
  cmd.AddValue ("packet-size",  "Size of packets in UDP ping (bytes)", m_packetSize);
  cmd.AddValue ("interfaces", "Number of radio interfaces used by each mesh point", m_nIfaces);
  cmd.AddValue ("channels",   "Use different frequency channels for different interfaces", m_chan);
  cmd.AddValue ("pcap",   "Enable PCAP traces on interfaces", m_pcap);
  cmd.AddValue ("ascii",   "Enable Ascii traces on interfaces", m_ascii);
  cmd.AddValue ("stack",  "Type of protocol stack. ns3::Dot11sStack by default", m_stack);
  cmd.AddValue ("root", "Mac address of root mesh point in HWMP", m_root);

  cmd.Parse (argc, argv);
  NS_LOG_DEBUG ("Grid:" << m_xSize << "*" << m_ySize);
  NS_LOG_DEBUG ("Simulation time: " << m_totalTime << " s");
  if (m_ascii)
    {
      PacketMetadata::Enable ();
    }
    */

  CommandLine cmd;
  cmd.Parse (argc, argv);

  NS_ASSERT_MSG(argc == 4, "The program expects 3 arguments.");

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UniformRoutingProtocol", LOG_LEVEL_ALL);
  LogComponentEnable ("UniformRoutingTable", LOG_LEVEL_ALL);
  LogComponentEnable ("WifiAdhocExperiment", LOG_LEVEL_ALL);
  
  LogComponentEnable ("UniformRoutingProtocol", LOG_LEVEL_ALL);
  LogComponentEnable ("UniformRequestQueue", LOG_LEVEL_ALL);
  //LogComponentEnable ("InternetStackHelper", LOG_LEVEL_ALL);
  
  //LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_ALL);
  //LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);

  //LogComponentEnable ("WifiPhy",LOG_LEVEL_ALL);
  //LogComponentEnable ("WifiMac",LOG_LEVEL_ALL);

  Packet::EnablePrinting ();

  // Disable the Icmpv6 Redirect on all nodes
  Config::SetDefault ("ns3::Ipv6L3Protocol::SendIcmpv6Redirect",  BooleanValue (false));

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (m_phyMode));
  
  // Get all the information from the arguments

  string str_currentExecDir(argv[1]);
  string str_posFile(argv[1]);
  str_posFile.append(".txt");
  string str_flowFile(argv[2]);

  m_numPackets = std::atoi(argv[3]);

  // copying the contents of the argument (string) to char array
  strcpy(m_currentExecDir, str_currentExecDir.c_str());
  strcpy(m_posFile, str_posFile.c_str());
  strcpy(m_flowFile, str_flowFile.c_str());
}

void
WifiAdhocExperiment::CreateNodes ()
{ 
  char definitePosFile[750];
  
  // Rough method for reading through the positions file
  sprintf (definitePosFile, "%s/%s/%s", m_experimentDir, m_currentExecDir, m_posFile);

  NS_LOG_INFO ("------Attempting to read positions file " <<  definitePosFile << " ------");

  // Attempt to read the file with the position data
  ifstream posFile;

  posFile.open (definitePosFile);

  if (!posFile.is_open ())
    {
      throw "ERROR: Error opening the node positions file.";
    }

  // Information to retrieve
  uint16_t x, y;
  string line;

  // Attempt to read everything
  getline(posFile, line);
  istringstream n_stream(line);
  n_stream >> m_n;
  NS_LOG_INFO ("Reading number of nodes : " << m_n);

  // Creating nodes
  nodes.Create(m_n);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (m_verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (0) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (m_rss));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (m_phyMode),
                                "ControlMode",StringValue (m_phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // Tracing
  wifiPhy.EnablePcap ("wifi-adhoc-experiment", devices);

  // Note that with FixedRssLossModel, the positions below are not
  // used for received signal strength.

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  for (int i=0; i<m_n; i++)
  {
    // Read the position of node i
    getline(posFile, line, ',');
    istringstream x_stream(line);
    x_stream >> x;

    getline(posFile, line);
    istringstream y_stream(line);
    y_stream >> y;

    NS_LOG_DEBUG ("Reading Node " << i << " at postion (" << x << ", "<< y << ")");

    positionAlloc->Add (Vector (float(x), float(y), 0.0));   
  }
  posFile.close();  
 
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
}

void
WifiAdhocExperiment::InstallInternetStack ()
{
  UniformHelper uniform; // TODO: make the routing protocol a parameter 
  InternetStackHelper internet;
  internet.SetRoutingHelper (uniform);
  internet.Install (nodes);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  interfaces = ipv6.Assign (devices);

  // The first address assignment was just to get the Ipv6interfaceContainer interfaces
  // I need an easily deduceable Ip address from the node index, thus the following assignment
  // TODO: See if the SetBase syntax can allow for the whole index to appear in the address
  for (uint8_t i=0; i< devices.GetN(); ++i) 
    {
      Ptr<NetDevice> netDev = devices.Get(i);
      Ptr<Node> n = netDev->GetNode();
      Ptr<Ipv6> ipv6Prot = n->GetObject<Ipv6>();

      int32_t ifindex = 0; 
      ifindex = ipv6Prot->GetInterfaceForDevice(netDev);

      int num = i + 2000; 

      std::stringstream stream;
      stream << num;
      stream << ":dbc:f00d:cafe::42"; 
      std::string newAdr(stream.str());    
   
      Ipv6InterfaceAddress IfAdr = Ipv6InterfaceAddress (Ipv6Address (newAdr.c_str()), Ipv6Prefix (64)); 
      ipv6Prot->AddAddress(ifindex, IfAdr);
    }

    // Set all nodes as routers
  for (uint32_t i=0; i< interfaces.GetN(); i++)
  {
    interfaces.SetForwarding(i, true);
  }
  // Tracing
  internet.EnablePcapIpv6All (std::string ("wifi-adhoc-experiment-ipv6"));
}

void
WifiAdhocExperiment::InstallApplication ()
{
  // Test with SensorInforTransferApp *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
  
  // Rough method for reading through the flows file
  char definiteFlowFile[750];
  sprintf (definiteFlowFile, "%s/%s/%s", m_experimentDir, m_currentExecDir, m_flowFile);

  NS_LOG_INFO ("------Attempting to read flows file " <<  definiteFlowFile << " ------");

  ifstream flowsFile;

  try
  {
    // Attempt to read the file with the selected flows
    flowsFile.open (definiteFlowFile);
    if (!flowsFile.is_open ())
      {
        throw "Couldn't open the traffic flows file.";
      }
    
    // Information to retrieve
    uint16_t srcId, dstId;
    
    uint16_t sink_port = 7887;
    string line;

    flowsFile.exceptions ( std::ifstream::eofbit ); 

    while (!flowsFile.eof())
    {
      getline(flowsFile, line, ' ');
      istringstream src_id_stream(line);
      src_id_stream >> srcId;

      getline(flowsFile, line);
      istringstream dst_id_stream(line);
      dst_id_stream >> dstId;

      NS_LOG_INFO ("------Installing the flow from Node "<< srcId 
        <<" to Node " << dstId << "------");

      SensorInfoTransferAppHelper oneFlow ("ns3::UdpSocketFactory",
            Inet6SocketAddress(interfaces.GetAddress (dstId, 2), sink_port));
      NS_LOG_INFO ("THE RECEIVER : " << interfaces.GetAddress (dstId, 2));
      NS_LOG_INFO ("THE SENDER : " << interfaces.GetAddress (srcId, 2));
             // In GetAddress, j = 0 (linklocal), 1 (first global), 2 (second global)
      
      // Make sure the application doesn't shut off before the end of the simulation
      oneFlow.SetAttribute("StopTime", TimeValue (Seconds(m_totalTime)));
      oneFlow.SetAttribute("Frequency", DoubleValue (1.0));
      oneFlow.SetAttribute("PacketNumber", IntegerValue (m_numPackets));

      oneFlow.Install (nodes.Get(srcId));
    }
  }
  catch (std::ifstream::failure &e)
  {
    if (flowsFile.eof())
      {
        NS_LOG_DEBUG ("Reached the end of the traffic flows file.");
        flowsFile.close();
      } 
    else throw "Exception opening/reading/closing source destination flows file.";
  }  
}

void
WifiAdhocExperiment::Run ()
{
  time (&m_begin); 
  CreateNodes ();
  InstallInternetStack ();
  InstallApplication ();
  AnimationInterface anim("wifi_adhoc_animation_wifi.xml");
  //Simulator::Schedule (Seconds (m_totalTime), &WifiAdhocExperiment::Report, this);
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
  time (&m_end);

  double difference = difftime (m_end, m_begin);
  NS_LOG_INFO (" Execution time: " << difference << " s");
}

void
WifiAdhocExperiment::Report ()
{
	// Do I need this? 

	// Output what we are doing
  NS_LOG_UNCOND ("Testing " /*<< numPackets */ << " packets sent with receiver rss " << m_rss );
  /*
  unsigned n (0);
  for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i, ++n)
    {
      std::ostringstream os;
      os << "mp-report-" << n << ".xml";
      std::cerr << "Printing mesh point device #" << n << " diagnostics to " << os.str () << "\n";
      std::ofstream of;
      of.open (os.str ().c_str ());
      if (!of.is_open ())
        {
          std::cerr << "Error: Can't open file " << os.str () << "\n";
          return;
        }
      mesh.Report (*i, of);
      of.close ();
    }
    */
}

int
main (int argc, char *argv[])
{
  WifiAdhocExperiment e; 
  try 
  {
  	e.Configure (argc, argv);
  	e.Run ();
    return 0;
  }
  catch (const char* msg)
  {
    NS_LOG_DEBUG ("In the catch block! Error " << msg);
  	return 1;
  }
}
