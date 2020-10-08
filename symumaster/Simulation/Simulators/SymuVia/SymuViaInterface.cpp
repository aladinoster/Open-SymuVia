#include "SymuViaInterface.h"

#include "Simulation/Simulators/TraficSimulatorHandler.h"

#include <Graph/Model/MultiLayersGraph.h>
#include <Graph/Model/Node.h>
#include <Graph/Model/Link.h>
#include <Graph/Model/NullPenalty.h>

#include <boost/log/trivial.hpp>

using namespace SymuMaster;
using namespace SymuCore;

SymuViaInterface::SymuViaInterface() :
    AbstractSimulatorInterface()
{

}

SymuViaInterface::SymuViaInterface(TraficSimulatorHandler * pSimulator, const std::string & strName, bool bUpstream) :
    AbstractSimulatorInterface(pSimulator, strName, bUpstream)
{
    // Le noeud correspond tout simplement � celui dont le nom est celui de l'interface
    const std::vector<Node*> & nodes = pSimulator->GetSimulationDescriptor().GetGraph()->GetListNodes();
    for (size_t iNode = 0; iNode < nodes.size(); iNode++)
    {
		Node * pNode = nodes[iNode];
		NodeType nodeType = pNode->getNodeType();
		if (pNode->getStrName() == strName && 
			// ce doit aussi �tre un type de noeud pour lequel une interface fait sens
			(nodeType == NT_RoadExtremity || nodeType == NT_Area || nodeType == NT_SubArea || nodeType == NT_PublicTransportStation || nodeType == NT_Parking)
			)
        {
            m_pNode = nodes[iNode];
            break;
        }
    }
}

bool SymuViaInterface::CanRestrictCapacity() const
{
	return m_pNode->getNodeType() == NT_RoadExtremity;
}

bool SymuViaInterface::CanComputeOffer() const
{
	return m_pNode->getNodeType() == NT_RoadExtremity;
}

bool SymuViaInterface::Check(std::vector<AbstractSimulatorInterface*> & associatedInterfaces) const
{
    // no particular constraint for symuvia interfaces
    return true;
}

void SymuViaInterface::BuildUpstreamPenalties(SymuCore::Pattern * pPatternToVirtualNode)
{
    // Pour SymuVia, on a ici une sortie ponctuelle qui est reli�e au noeud virtuel correspondant � l'interface.
    // Il faut donc ajouter une penalty permettant de passer de tous les patterns amont � la sortie ponctuelle
    // vers le pattern d'interface aval � la sortie pass� en argument :
    Node * pExitNode = pPatternToVirtualNode->getLink()->getUpstreamNode();
    for (size_t iUpLink = 0; iUpLink < pExitNode->getUpstreamLinks().size(); iUpLink++)
    {
        Link * pUpLink = pExitNode->getUpstreamLinks()[iUpLink];

        for (size_t iUpPattern = 0; iUpPattern < pUpLink->getListPatterns().size(); iUpPattern++)
        {
            Pattern * pUpPattern = pUpLink->getListPatterns()[iUpPattern];

            AbstractPenalty * pPenalty = new NullPenalty(pExitNode, PatternsSwitch(pUpPattern, pPatternToVirtualNode));
            pExitNode->getMapPenalties()[pUpPattern][pPatternToVirtualNode] = pPenalty;
        }
    }
}

void SymuViaInterface::BuildDownstreamPenalties(SymuCore::Pattern * pPatternFromVirtualNode)
{
    // Pour SymuVia, on a ici une entr�e ponctuelle qui est reli�e au noeud virtuel correspondant � l'interface.
    // Il faut donc ajouter une penalty permettant de passer depuis le pattern d'interface amont pass� en argument
    // � tous les patterns aval � l'entr�e ponctuelle :
    Node * pEntryNode = pPatternFromVirtualNode->getLink()->getDownstreamNode();
    for (size_t iDownLink = 0; iDownLink < pEntryNode->getDownstreamLinks().size(); iDownLink++)
    {
        Link * pDownLink = pEntryNode->getDownstreamLinks()[iDownLink];

        for (size_t iDownPattern = 0; iDownPattern < pDownLink->getListPatterns().size(); iDownPattern++)
        {
            Pattern * pDownPattern = pDownLink->getListPatterns()[iDownPattern];

            AbstractPenalty * pPenalty = new NullPenalty(pEntryNode, PatternsSwitch(pPatternFromVirtualNode, pDownPattern));
            pEntryNode->getMapPenalties()[pPatternFromVirtualNode][pDownPattern] = pPenalty;
        }
    }
}
