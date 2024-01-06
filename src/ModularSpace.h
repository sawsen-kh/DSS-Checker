// ModularSpace.h: interface for the ModularSpace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODULARSPACE_H__8D2658DD_5CFB_4D6D_9491_854AE66CF78E__INCLUDED_)
#define AFX_MODULARSPACE_H__8D2658DD_5CFB_4D6D_9491_854AE66CF78E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Automata.h"
#include "ModularPetriNet.h"
#include "NodeSGSimplified.h"

class ModularPetriNet;

class Automata;

class ModularSpace {
    friend class Automata;

public:


    long getLocalArcsCount();

    long getNodesCount();

    long getTotalNodesCount();

    long getModulesCount();

    ModularSpace();

    virtual ~ModularSpace();

    string getCorrespondant(ListMarquage *noeud, const int index_module);

    string getCorrespondantMarquage(Marking marq, const int index);

    string getCorrespondantNodeSG(NodeSGSimplified *node);

private:


    ModularPetriNet *m_petri;
    vector<Automata *> m_automates;
    GrapheSync *m_graphe_sync;

};

#endif // !defined(AFX_MODULARSPACE_H__8D2658DD_5CFB_4D6D_9491_854AE66CF78E__INCLUDED_)
