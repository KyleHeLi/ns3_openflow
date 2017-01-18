/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Network topology
//
//      -----------
//      |  Switch |
//      -----------
//      |  |  | | |
//      /  |  | | \
//     /   |  | |  \
//     s1  | s2 |   s3
//         |    |
//         /    \
//       s4      s5
//
//  s1----s2-----s3
//  |\     |     /
//  | \    |    /
//  |  \   |   /
//  |   \  |  /
//  s4-----s5
//
//
//

bool verbose = false;
bool use_drop = true;


int main( int argc , char *argv[]){
#ifdef NS3_OPENFLOW

	CommandLine cmd;

// create s1-5
	NS_LOG_INFO("Create nodes.");
	NodeContainer terminals;
	terminals.Create(5);

//create switch
	NodeContainer csmaSwitch;
	csmaSwitch.Create(1);

//setting switch parameter
	NS_LOG_INFO("Build Topology");
	csmaHelper csma;
	csma.SetChannelAttribute("DateRate" , DataRateValue(5000000));
	csma.SetChannelAttribute("Delay" , TimeValue(MilliSeconds(2)));

//create link from s1-5 to switch
	NetDeviceContainer terminalDevices;
	NetDeviceContainer switchDevices;

	for( int i=0; i<5; i++){
		NetDeviceContainer link = csma.Install(NodeContainer(terminals.Get(i), csmaSwitch));
		terminalDevices.Add(link.Get(0));
		switchDevices.Add(link.Get(1));
		//todo: check if all link.Get(1) is same
	}

//Create link between s1-s5
	NodeContainer p2pNode1;//s1-s2
	NodeContainer p2pNode2;//s2-s3
	NodeContainer p2pNode3;//s1-s4
	NodeContainer p2pNode4;//s1-s5
	NodeContainer p2pNode5;//s2-s5
	NodeContainer p2pNode6;//s3-s5
	NodeContainer p2pNode7;//s4-s5
	p2pNode1.Add()


//Create the switch netdevice, which will do the packet switching
	Ptr<Node> switchNode = csmaSwitch.Get(0);
	OPenFlowSwitchHelper OFSwitch;


	Ptr<ns3::ofi::DropController> controller = CreateObject<ns3::ofi::DropController> ();
	OFSwitch.Install( switchNode, switchDevices , controller);

//Add internet stack to the terminals
	InternetStackHelper internet;
	internet.Install(terminals);



#else
	NS_LOG_INFO("NS3 Openflow is not enabled. Cannot run simulation.");
#endif

	return 0;
}
