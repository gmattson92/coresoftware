#include "InttRawDataDecoder.h"
#include "InttMapping.h"

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/getClass.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHNodeIterator.h>

#include <trackbase/InttDefs.h>
#include <trackbase/TrkrHit.h>
#include <trackbase/TrkrHitv2.h>
#include <trackbase/TrkrHitSet.h>
#include <trackbase/TrkrHitSetContainer.h>
#include <trackbase/TrkrHitSetContainerv1.h>

InttRawDataDecoder::InttRawDataDecoder(std::string const& name):
	SubsysReco(name)
{
	//Do nothing
}

//Init
int InttRawDataDecoder::Init(PHCompositeNode*)
{
	//Do nothing
	return Fun4AllReturnCodes::EVENT_OK;
}

//InitRun
int InttRawDataDecoder::InitRun(PHCompositeNode* topNode)
{
	if(!topNode)
	{
		std::cout << "InttRawDataDecoder::InitRun(PHCompositeNode* topNode)" << std::endl;
		std::cout << "\tCould not retrieve topNode; doing nothing" << std::endl;

		return -1;
	}

	PHNodeIterator dst_itr(topNode);
	PHCompositeNode* dst_node = dynamic_cast<PHCompositeNode*>(dst_itr.findFirst("PHCompositeNode", "DST"));
	if(!dst_node)
	{
		if(Verbosity())std::cout << "InttRawDataDecoder::InitRun(PHCompositeNode* topNode)" << std::endl;
		if(Verbosity())std::cout << "\tCould not retrieve dst_node; doing nothing" << std::endl;

		return -1;
	}

	PHNodeIterator trkr_itr(dst_node);
	PHCompositeNode* trkr_node = dynamic_cast<PHCompositeNode*>(trkr_itr.findFirst("PHCompositeNode", "TRKR"));
	if(!trkr_node)
	{
		trkr_node = new PHCompositeNode("TRKR");
		dst_node->addNode(trkr_node);
	}

	TrkrHitSetContainer* trkr_hit_set_container = findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");
	if(!trkr_hit_set_container)
	{
		if(Verbosity())std::cout << "InttRawDataDecoder::InitRun(PHCompositeNode* topNode)" << std::endl;
		if(Verbosity())std::cout << "\tMaking TrkrHitSetContainer" << std::endl;

		trkr_hit_set_container = new TrkrHitSetContainerv1;
		PHIODataNode<PHObject>* new_node = new PHIODataNode<PHObject>(trkr_hit_set_container, "TRKR_HITSET", "PHObject");
		trkr_node->addNode(new_node);
	}

	return Fun4AllReturnCodes::EVENT_OK;
}

//process_event
int InttRawDataDecoder::process_event(PHCompositeNode* topNode)
{
	TrkrHitSetContainer* trkr_hit_set_container = findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");
	if(!trkr_hit_set_container)
	{
		std::cout << PHWHERE << std::endl;
		std::cout << "InttRawDataDecoder::process_event(PHCompositeNode* topNode)" << std::endl;
		std::cout << "Could not get \"TRKR_HITSET\" from Node Tree" << std::endl;
		std::cout << "Exiting" << std::endl;
		exit(1);

		return Fun4AllReturnCodes::DISCARDEVENT;
	}


	Event* evt = findNode::getClass<Event>(topNode, "PRDF");
	if(!evt)return Fun4AllReturnCodes::DISCARDEVENT;

	struct Intt::RawData_s rawdata;
	struct Intt::Offline_s offline;

	int adc = 0;
	//int amp = 0;
	int bco = 0;

	TrkrDefs::hitsetkey hit_set_key = 0;
	TrkrDefs::hitkey hit_key = 0;
	TrkrHitSetContainer::Iterator hit_set_container_itr;
	TrkrHit* hit = nullptr;

	for(std::map<int, int>::const_iterator itr = Intt::Packet_Id.begin(); itr != Intt::Packet_Id.end(); ++itr)
	{
		Packet* p = evt->getPacket(itr->first);
		if(!p)continue;

		int N = p->iValue(0, "NR_HITS");
		full_bco = p->lValue(0, "BCO");

		if(Verbosity() > 20)std::cout << N << std::endl;

		for(int n = 0; n < N; ++n)
		{
			rawdata = Intt::RawFromPacket(itr->second, n, p);

			adc = p->iValue(n, "ADC");
			//amp = p->iValue(n, "AMPLITUE");
			bco = p->iValue(n, "FPHX_BCO");

			offline = Intt::ToOffline(rawdata);

			hit_key = InttDefs::genHitKey(offline.strip_x, offline.strip_y);
			hit_set_key = InttDefs::genHitSetKey(offline.layer, offline.ladder_z, offline.ladder_phi, bco);

			hit_set_container_itr = trkr_hit_set_container->findOrAddHitSet(hit_set_key);
			hit = hit_set_container_itr->second->getHit(hit_key);
			if(hit)continue;

			hit = new TrkrHitv2;
			hit->setAdc(adc);
			hit_set_container_itr->second->addHitSpecificKey(hit_key, hit);
		}

		delete p;
	}

	if(Verbosity() > 20)
	{
		std::cout << std::endl;
		std::cout << "Identify():" << std::endl;
		trkr_hit_set_container->identify();
		std::cout << std::endl;
	}

	return Fun4AllReturnCodes::EVENT_OK;
}

//End
int InttRawDataDecoder::End(PHCompositeNode* /*topNode*/)
{
	//Do nothing
	return Fun4AllReturnCodes::EVENT_OK;
}

