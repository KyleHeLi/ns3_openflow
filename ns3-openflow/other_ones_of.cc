/*from [http://blog.csdn.net/u012174021/article/details/42320033]

*/
/*
n0-------of1-----of1------n3
          |       |
          |       |
          n1      n2

network topology
*/

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/openflow-module.h"
#include "ns3/log.h"
#include "ns3/bridge-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OpenFlowCsmaSwitch");

bool verbose = false;
bool use_drop = false;
ns3::Time timeout = ns3::Seconds (0);

bool
SetVerbose (std::string value)
{
	  verbose = true;
	    return true;
}

bool
SetDrop (std::string value)
{
	  use_drop = true;
	    return true;
}

bool
SetTimeout (std::string value)
{
	  try {
		        timeout = ns3::Seconds (atof (value.c_str ()));
				      return true;
					      }
	    catch (...) { return false; }
		  return false;
}

int
main(int argc,char *argv[])
{
	CommandLine cmd;

	cmd.AddValue ("v", "Verbose (turns on logging).", MakeCallback (&SetVerbose));
	cmd.AddValue ("verbose", "Verbose (turns on logging).", MakeCallback (&SetVerbose));
	cmd.AddValue ("d", "Use Drop Controller (Learning if not specified).", MakeCallback (&SetDrop));
	cmd.AddValue ("drop", "Use Drop Controller (Learning if not specified).", MakeCallback (&SetDrop));
	cmd.AddValue ("t", "Learning Controller Timeout (has no effect if drop controller is specified).", MakeCallback ( &SetTimeout));
	cmd.AddValue ("timeout", "Learning Controller Timeout (has no effect if drop controller is specified).", MakeCallback ( &SetTimeout));

	cmd.Parse (argc, argv);

	if (verbose)
	{
		LogComponentEnable ("OpenFlowCsmaSwitch", LOG_LEVEL_INFO);
		LogComponentEnable ("OpenFlowInterface", LOG_LEVEL_INFO);
		LogComponentEnable ("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
		LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	NS_LOG_INFO ("Create nodes");
	
	NodeContainer terminals;
	terminals.Create(4);

	NodeContainer ofSwitch;
	ofSwitch.Create(2);

	NS_LOG_INFO ("Create Topology");
	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate",DataRateValue(5000000));
	csma.SetChannelAttribute("Delay",TimeValue(MilliSeconds(2)));

	NetDeviceContainer terminalDevices;
	NetDeviceContainer switchDevice1,switchDevice2;
	NetDeviceContainer link;

	//Connect ofSwitch1 to ofSwitch2
	link = csma.Install(NodeContainer(ofSwitch.Get(0),ofSwitch.Get(1)));
	switchDevice1.Add(link.Get(0));
	switchDevice2.Add(link.Get(1));

	//Connect terminal1 and terminal2 to ofSwitch1
	for (int i = 0;i < 2;i ++)
	{
		link = csma.Install(NodeContainer(terminals.Get(i),ofSwitch.Get(0)));
		terminalDevices.Add(link.Get(0));
		switchDevice1.Add(link.Get(1));
	}

	//Connect terminal3 and terminal4 to ofSwitch2
	for (int i = 2;i < 4;i ++)
	{
		link = csma.Install(NodeContainer(terminals.Get(i),ofSwitch.Get(1)));
		terminalDevices.Add(link.Get(0));
		switchDevice2.Add(link.Get(1));
	}

	//Create the switch netdevice,which will do the packet switching
	Ptr<Node> OFNode1 = ofSwitch.Get(0);
	Ptr<Node> OFNode2 = ofSwitch.Get(1);

	OpenFlowSwitchHelper ofswHelper;
	if (use_drop)
	{
		Ptr<ns3::ofi::DropController> controller1 = CreateObject<ns3::ofi::DropController> ();
		ofswHelper.Install (OFNode1, switchDevice1, controller1);
		Ptr<ns3::ofi::DropController> controller2 = CreateObject<ns3::ofi::DropController> ();
		ofswHelper.Install (OFNode2, switchDevice2, controller2);
	}
	else
	{
		Ptr<ns3::ofi::LearningController> controller1 = CreateObject<ns3::ofi::LearningController> ();
		if (!timeout.IsZero ()) controller1->SetAttribute ("ExpirationTime", TimeValue (timeout));
		ofswHelper.Install (OFNode1, switchDevice1, controller1);
		Ptr<ns3::ofi::LearningController> controller2 = CreateObject<ns3::ofi::LearningController> ();
		if (!timeout.IsZero ()) controller2->SetAttribute ("ExpirationTime", TimeValue (timeout));
		ofswHelper.Install (OFNode2, switchDevice2, controller2);
	}

	//Add internet stacks to the terminals
	InternetStackHelper internet;
	internet.Install (terminals);

	//Add IP addresses
	NS_LOG_INFO ("Assign IP Addresses.");
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interface = ipv4.Assign(terminalDevices);

	//Add applications
	NS_LOG_INFO ("Create an application");
	UdpEchoServerHelper echoServer (9);
	ApplicationContainer serverApps = echoServer.Install (terminals.Get(0));
	serverApps.Start (Seconds(1.0));
	serverApps.Stop (Seconds(10.0));

	UdpEchoClientHelper echoClient (Ipv4Address("10.1.1.1"),9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
	ApplicationContainer clientApps = echoClient.Install(terminals.Get(3));
	clientApps.Start (Seconds(2.0));
	clientApps.Stop (Seconds(10.0));
	
	AsciiTraceHelper ascii;
	csma.EnableAsciiAll (ascii.CreateFileStream ("openflow-switch.tr"));

	csma.EnablePcapAll ("openflow-switch", false);

	NS_LOG_INFO ("Run simulation");
	Simulator::Run();
	Simulator::Destroy();
	NS_LOG_INFO ("Done.");
	//#else
	//NS_LOG_INFO ("NS-3 OpenFlow is not enabled. Cannot run simulation.");
	//#endif // NS3_OPENFLOW

}