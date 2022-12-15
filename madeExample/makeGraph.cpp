//STD
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

//mkdir
#include <sys/types.h>
#include <sys/stat.h>

//graphviz library
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
#include <graphviz/cdt.h>

bool printGraph(Agraph_t *);
void setOutput(GVC_t *, Agraph_t *, bool);

int MAX_NODE = 100;

int main(int argc, char **argv) 
{
  if (argc > 1)
    if(sscanf(argv[1], "%d", &MAX_NODE) == -1 || MAX_NODE <= 0)
      MAX_NODE = 1000;

  GVC_t *gvc = gvContext();

  //Creo grafo out
  string nomeGrafo = "EsempioConNodi" + to_string(MAX_NODE);
  Agraph_t *graphOut = agopen(nomeGrafo.c_str(), Agdirected, NULL);
  Agnode_t* startNode = agnode(graphOut, to_string(0).c_str(), TRUE);
  agsafeset(startNode, "label", to_string(0).c_str(), "");
  Agedge_t *startEdge = agedge(graphOut, startNode, startNode, "eps", TRUE);
  agsafeset(startEdge, "label", "eps", "");

  //Setto le proprietà del grafo
  agsafeset(startNode, "shape", "circle", "circle");
  agsafeset(startNode, "style", "filled", "filled");
  agsafeset(startNode, "fontname", "courier", "courier");
  agsafeset(startNode, "colorscheme", "paired6", "paired6");
  agsafeset(startNode, "fillcolor", "1", "1");
  agsafeset(startEdge, "fontname", "courier", "courier");

  random_device rd;
  uniform_int_distribution<int> randomNode(0, MAX_NODE-1);
  uniform_int_distribution<int> probThird(0, 2);
  uniform_int_distribution<int> probQuarter(0, 3);
  
  for(int i = 0; i < MAX_NODE; i++)
  { 
    //Creo nodo nuovo
    Agnode_t* node = agnode(graphOut, to_string(i).c_str(), TRUE);
    agsafeset(node, "label", to_string(i).c_str(), "");

    //Decido se è stato finale
    if(probQuarter(rd) == 0)
      agset(node, "shape", "doublecircle");

    bool hasTransitions = false;

    //Decido se ha a tran
    if(probThird(rd) == 0)
    {
      Agnode_t* otherNode = agnode(graphOut, to_string(randomNode(rd)).c_str(), TRUE);
      Agedge_t *edge = agedge(graphOut, node, otherNode, "a", TRUE);
      agsafeset(edge, "label", "a", "");
      hasTransitions = true;
    }

    //Decido se ha b tran
    if(probThird(rd) == 0)
    {
      Agnode_t* otherNode = agnode(graphOut, to_string(randomNode(rd)).c_str(), TRUE);
      Agedge_t *edge = agedge(graphOut, node, otherNode, "b", TRUE);
      agsafeset(edge, "label", "b", "");
      hasTransitions = true;
    }

    //Decido se ha Epsilon tran
    if(probThird(rd) == 0 || !hasTransitions)
    {
      Agnode_t* otherNode = agnode(graphOut, to_string(randomNode(rd)).c_str(), TRUE);
      Agedge_t *edge = agedge(graphOut, node, otherNode, "eps", TRUE);
      agsafeset(edge, "label", "eps", "");
    }
  }

  //Stampo le caratt del grafo in output
  //cout << "Output: " << endl;
  //printGraph(graphOut);

  //Converto e stampo i file di output
  setOutput(gvc, graphOut, false);

  //Chiudo libero memoria grafo out
  gvFreeLayout(gvc, graphOut);
  agclose(graphOut);

  return gvFreeContext(gvc);
}

void setOutput(GVC_t *gvc, Agraph_t *graphOut, bool printImage)
{
  //Creo cartella out
  mkdir("out", 0777);

  //Converto e visualizzo il grafo out
  gvLayout(gvc, graphOut, "dot");
  string nnodi = to_string(MAX_NODE);
  string nome = "out/graph" + nnodi;
  string dotstr = nome + ".dot";
  //string jpgstr = nome + ".jpg";
  gvRenderFilename (gvc, graphOut, "dot", dotstr.c_str());
  //gvRenderFilename (gvc, graphOut, "jpg", jpgstr.c_str());

  //Visualizzo immagine
  if(printImage)
    system("fim out/graphOUT.png &");
}


bool printGraph(Agraph_t *g)
{
  //Info su grafo
  cout << "Grafo " << (agnameof(g) == NULL ? "NULL" : agnameof(g)) << " :";
  cout << " V(" << agnnodes(g) << ")";
  cout << " E(" << agnedges(g) << ")";
  cout << endl << endl;

  for (Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n)) 
	{
    //Info su nodo
    cout << "Nodo " << agnameof(n) << " ";
    if(agget(n,"label") != NULL)
      cout << "(" << agget(n,"label") << ")";
    cout << ":";
    cout << " EOut(" << agdegree(g,n,FALSE,TRUE) << ")";
    cout << " EIn(" << agdegree(g,n,TRUE,FALSE) << ")";
    if(agget(n, "shape") != NULL)
    {
      string tmp(agget(n,"shape"));
      if(tmp == "doublecircle")
        cout << " FINALE";
    }
    cout << endl;
		for (Agedge_t *e = agfstout(g,n); e; e = agnxtout(g,e)) 
		{
      cout << "Arco ";
      if(agnameof(e) != NULL)
        cout << "(" << agnameof(e) << ")";
      cout << "--> " << agnameof(e->node) << " (" << agget(e, "label") << ")";
      cout << endl;
		}
    cout << endl;
	}

  return true;
}