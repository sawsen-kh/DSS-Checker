//
// Created by chiheb on 13/07/22.
//

#include "DSSBuilder.h"
#include "ArcSync.h"
#include <fstream>

DSSBuilder::DSSBuilder(ModularPetriNet *ptr) : mptrMPNet(ptr) {
    for (uint32_t i = 0; i < mptrMPNet->getNbModules(); i++) {
        auto elt = new ModuleSS();
        mlModuleSS.push_back(elt);
    }
}

/*
 * Build the DSS
 */
void DSSBuilder::build() {
    buildInitialMS();
}

/*
 * Build initial MS
 */
void DSSBuilder::buildInitialMS() {
    MetaState *ms;
    // Build initial meta-states
    cout << "Build initial meta-states" << endl;
    ProductSCC *productscc = new ProductSCC();
    vector<MetaState *> list_metatstates;
    for (int module = 0; module < mptrMPNet->getNbModules(); ++module) {
        ms = new MetaState();

        StateGraph *state_graph = mptrMPNet->getModule(module)->getStateGraph(
                mptrMPNet->getModule(module)->getMarquage());
        state_graph->setID(module);
        ms->setStateGraph(state_graph);
        mlModuleSS[module]->insertMS(ms);
        list_metatstates.push_back(ms);
        productscc->addSCC(ms->getInitialSCC());
    }
    // Setting name for meta-states
    for (int i = 0; i < mptrMPNet->getNbModules(); ++i) {
        list_metatstates.at(i)->setSCCProductName(productscc);
    }

    vector<RElement_dss> list_fusions;
    mptrMPNet->extractEnabledFusionReduced(list_metatstates, list_fusions);

    vector<RListProductFusion> stack_fusion;
    stack_fusion.push_back(list_fusions);

    while (!stack_fusion.empty()) {
        list_fusions = stack_fusion.back();
        stack_fusion.pop_back();
        for (int i = 0; i < list_fusions.size(); i++) {
            RElement_dss elt = list_fusions.at(i);
            for (int index_global_state = 0;
                 index_global_state < elt.m_gs->size();
                 index_global_state++) {
                vector<Marking *> *global_state = elt.m_gs->at(
                        index_global_state);
                for (int module = 0; module < mptrMPNet->getNbModules(); module++) {
                    mptrMPNet->getModule(module)->setMarquage(global_state->at(module));
                }
                Fusion *fusion = elt.m_fusion;
                fusion->tirer();

                // Build obtained destination Meta-states and check whether they exist or not
                MetaState *dest_ms;
                // Build initial meta-states
                //cout << "Build destination meta-states" << endl;
                ProductSCC *dest_productscc = new ProductSCC();
                vector<MetaState *> dest_list_metatstates;
                dest_list_metatstates.resize(mptrMPNet->getNbModules());
                for (int module = 0; module < mptrMPNet->getNbModules(); module++) {
                    if (fusion->participate(module)) {
                        dest_ms = new MetaState();

                        StateGraph *state_graph =
                                mptrMPNet->getModule(module)->getStateGraph(
                                        mptrMPNet->getModule(module)->getMarquage());
                        state_graph->setID(module);

                        dest_ms->setStateGraph(state_graph);

                        dest_list_metatstates[module] = dest_ms;
                        dest_productscc->addSCC(dest_ms->getInitialSCC());
                    } else
                        dest_productscc->addSCC(
                                elt.getMetaState(module)->getSCCProductName()->getSCC(
                                        module));

                }

                // Compute the start product
                ProductSCC startProduct;
                for (int i = 0; i < global_state->size(); i++) {
                    MetaState *_source_ms = elt.getMetaState(i);
                    SCC *_scc = _source_ms->findSCC(global_state->at(i));
                    startProduct.addSCC(_scc);
                }
                // Check whether the computed product SCC exists or not

                vector<MetaState *> list_dest_metatstates;
                bool exist = false;
                for (int module = 0; module < mptrMPNet->getNbModules(); module++) {
                    if (fusion->participate(module)) {
                        if (!mlModuleSS[module]->findMetaStateByProductSCC(
                                *dest_productscc)) {
                            ArcSync *arc_sync = new ArcSync();
                            MetaState *source_ms = elt.getMetaState(module);
                            arc_sync->setData(startProduct, fusion,
                                              dest_list_metatstates.at(module));
                            source_ms->addSyncArc(arc_sync);
                            //dest_list_metatstates.at(module)->addSyncArc(arc_sync);
                            mlModuleSS[module]->insertMS(dest_list_metatstates.at(module));

                            //m_modules[module]->printMetaStateEx(
                            //		dest_list_metatstates.at(module));
                            dest_list_metatstates.at(module)->setSCCProductName(dest_productscc);
                            list_dest_metatstates.push_back(dest_list_metatstates.at(module));
                        } else {
                            exist = true;
                            ArcSync *arc_sync = new ArcSync();
                            MetaState *source_ms = elt.getMetaState(module);
                            MetaState *ms_dest = mlModuleSS[module]->findMetaStateByProductSCC(*dest_productscc);
                            arc_sync->setData(startProduct, fusion, ms_dest);
                            source_ms->addSyncArc(arc_sync);
                            list_dest_metatstates.push_back(ms_dest);

                        }

                    } else
                        list_dest_metatstates.push_back(
                                elt.getMetaState(module));

                }

                if (!exist) {
                    vector<RElement_dss> new_list_fusions;
                    mptrMPNet->extractEnabledFusionReduced(list_dest_metatstates, new_list_fusions);
                    stack_fusion.push_back(new_list_fusions);
                }
            }
        }
    }
}


void DSSBuilder::writeToFile(const string &filename) {
    std::ofstream myfile;
    myfile.open(filename);
    myfile << "digraph " << "fichier " << "{" << endl;
    myfile << "compound=true" << endl;
    for (int module = 0; module < mptrMPNet->getNbModules(); ++module) {
        PetriNet *petri = mptrMPNet->getModule(module);
        for (int i = 0; i < mlModuleSS[module]->getMetaStateCount(); i++) {
            MetaState *ms = mlModuleSS[module]->getMetaState(i);
            ProductSCC *pscc = ms->getSCCProductName();
            myfile << "subgraph cluster" << getProductSCCName(pscc) << module
                   << " {" << endl;
            /*********************************/
            StateGraph *ss=ms->getStateGraph();
            for (int jj=0;jj<ss->getCountArcs();++jj) {
                Marking *source=ss->getListMarquages()->at(jj);
                auto sourceName=petri->getMarquageName(*source);
                myfile<<sourceName;
                auto lsucc=source->getListSucc();
                for (const auto & elt : *lsucc) {

                }
            }
            /***********************************/
            /*for (int jj = 0; jj < ms->getListArcs()->size(); jj++) {
                Marking *source_marq = ms->getListArcs()->at(jj).getSource();
                Marking *dest_marq =
                        ms->getListArcs()->at(jj).getDestination();
                myfile << petri->getMarquageName(*source_marq)
                        << getProductSCCName(pscc) << module << " -> ";
                myfile << petri->getMarquageName(*dest_marq)
                        << getProductSCCName(pscc) << module;
                myfile << " [label=\""
                        << ms->getListArcs()->at(jj).getTransition()->getName()
                        << "\"]" << endl;
            }*/
            myfile << "label=\"" << getProductSCCName(pscc) << "\"" << endl;
            myfile << "}" << endl;

            for (int k = 0; k < ms->getSucc().size(); k++) {
                myfile << petri->getSCCName(pscc->getSCC(module))
                       << getProductSCCName(pscc) << module << " -> ";
                ArcSync *arc = ms->getSucc().at(k);
                MetaState *ms_dest = arc->getMetaStateDest();
                myfile
                        << petri->getSCCName(
                                ms_dest->getSCCProductName()->getSCC(module))
                        << getProductSCCName(ms_dest->getSCCProductName())
                        << module;
                myfile << " [ltail=cluster" << getProductSCCName(pscc) << module
                       << ",lhead=cluster"
                       << getProductSCCName(ms_dest->getSCCProductName())
                       << module << "]" << endl;
                //myfile<<arc->getFusion()->getName()<<"]";
                //myfile<<endl;

            }
        }
    }
    myfile << "}" << endl;
    myfile.close();
}

string DSSBuilder::getProductSCCName(ProductSCC *pss) {
    string res = "";
    for (int module = 0; module < mptrMPNet->getNbModules(); ++module) {
        PetriNet *petri = mptrMPNet->getModule(module);
        res += petri->getSCCName(pss->getSCC(module));
    }
    return res;
}