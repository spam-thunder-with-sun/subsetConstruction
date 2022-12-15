//STD
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset> 
#include <chrono>
using namespace std;
using namespace std::chrono;

//graphviz library
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
#include <graphviz/cdt.h>

void subsetConstruction(Agraph_t *, Agraph_t *);
bool printGraph(Agraph_t *);
void setOutput(GVC_t *, Agraph_t *, bool);
void setInput(GVC_t *, Agraph_t *, bool);

const int MAX_NODE = 1000;

bitset<MAX_NODE> epsClosure(Agraph_t*, const bitset<MAX_NODE>);
bitset<MAX_NODE> alphaClosure(Agraph_t *, const bitset<MAX_NODE>, char);
bool isFinal(Agraph_t*, const bitset<MAX_NODE>);
void subsetConstruction(Agraph_t*, Agraph_t *);

int main(int argc, char **argv) 
{
  assert (argc > 1);

  //Apertura contesto
  GVC_t *gvc = gvContext();

  //Apertura file
  FILE *dotFile = fopen(argv[1], "r");

  //Creazione grafo da file .dot
  Agraph_t *graphIn = agread(dotFile, NULL);

  //Chiusura file
  fclose(dotFile);

  //Creo grafo out
  Agraph_t *graphOut = agopen("Output", Agdirected, NULL);
  //Creo nodo ed arco nel grafo di uscita per settare le proprietà e poi gli elimino
  {
    Agnode_t *node = agnode(graphOut, "foo", TRUE);
    Agedge_t *edge = agedge(graphOut, node, node, "quu", TRUE);
    //Setto le proprietà del grafo
    agsafeset(node, "shape", "circle", "circle");
    agsafeset(node, "style", "filled", "filled");
    agsafeset(node, "fontname", "courier", "courier");
    agsafeset(node, "colorscheme", "paired6", "paired6");
    agsafeset(node, "fillcolor", "1", "1");
    agsafeset(edge, "fontname", "courier", "courier");
    //Elimino il nodo e l'arco
    agdeledge(graphOut, edge);
    agdelnode(graphOut, node);
  }

  high_resolution_clock::time_point start, finish;
  microseconds time_span;

  //Computo Subset Construction
  cout << "Computazione Subset Construction in corso ...";
  fflush(stdout);
  start = high_resolution_clock::now();
  subsetConstruction(graphIn, graphOut);
  finish = high_resolution_clock::now();
  time_span = duration_cast<microseconds>(finish-start);
  cout << "DONE (" << time_span.count() << "µs)" << endl;

  //Chiudo libero memoria grafo in
  gvFreeLayout(gvc, graphIn);
  agclose(graphIn);

  //Converto e stampo i file di output
  cout << "Creazione dot file e svg file...";
  fflush(stdout);
  start = high_resolution_clock::now();
  gvLayout(gvc, graphOut, "dot");
  gvRenderFilename (gvc, graphOut, "dot", "output.dot");
  gvRenderFilename (gvc, graphOut, "svg", "output.svg");
  //gvRenderFilename (gvc, graphOut, "png", "output.png");
  finish = high_resolution_clock::now();
  time_span = duration_cast<microseconds>(finish-start);
  cout << "DONE (" << time_span.count() << "µs)" << endl;

  //Viusualizzo immagine
  //system("fim output.png &");

  //Chiudo libero memoria grafo out
  gvFreeLayout(gvc, graphOut);
  agclose(graphOut);

  return gvFreeContext(gvc);
}

void subsetConstruction(Agraph_t *graphIn, Agraph_t *graphOut)
{
  //Varibili di supporto
  bitset<MAX_NODE> tmpBitset;

  //Vettore "Tabella"
  vector<bitset<MAX_NODE>> statusSet;
  statusSet.push_back({});

  //Set per la ricerca di insiemi di stati e l'iteratore per la ricerca
  unordered_map<bitset<MAX_NODE>, long, hash<bitset<MAX_NODE>>> statusSetFind;
  unordered_map<bitset<MAX_NODE>, long, hash<bitset<MAX_NODE>>>::const_iterator got;

  //Vettore alpha simboli
  char alphaSymbol[2] = {'a', 'b'};

  //Inserisco nodo di partenza
  statusSet[0][0] = 1;
  //Calcolo epsilon chiusura del nodo di partenza
  statusSet[0] = epsClosure(graphIn, statusSet[0]);
  //Inserisco nodo di partenza nell'hashmap
  statusSetFind.insert(make_pair(statusSet[0], 0));

  //Creo nodo nel grafo di uscita e setto la label
  Agnode_t * startNode = agnode(graphOut, to_string(0).c_str(), TRUE);
  agsafeset(startNode, "label", to_string(0).c_str(), "");

  //Se contiene nodi finali allora è uno stato finale
  if(isFinal(graphIn, statusSet[0]))
    agset(startNode, "shape", "doublecircle");

  for(long i = 0; i < statusSet.size(); ++i)
  {
    //Per ogni alphasimbolo
    for(char alpha : alphaSymbol)
    {
      //Calcolo alpha chiusura
      tmpBitset =  alphaClosure(graphIn, statusSet[i], alpha);
      if(tmpBitset.any())
      {
        //Calcolo epsilon chiusura
        tmpBitset = epsClosure(graphIn, tmpBitset);
        if(tmpBitset.any())
        {   
          //Prendo il nodo attuale per creare un arco
          Agnode_t* nowNode = agnode(graphOut, to_string(i).c_str(), FALSE);
          //Prendo l'altro nodo per creare un arco
          Agnode_t* otherNode;

          //Ricerca se esiste un bitset(insieme di stati) uguale
          //Nel caso medio in O(1) grazie all'hashmap
          got = statusSetFind.find(tmpBitset);
          if(got == statusSetFind.end())
          {
            //Se non esiste
            //Inserisco il nuovo stato nel vettore
            statusSet.push_back(tmpBitset);
            //Inserisco il nuovo stato nell'hashmap per la ricerca
            statusSetFind.insert(make_pair(tmpBitset, statusSet.size()-1));

            //Creo il nuovo nodo nel grafo di uscita e setto anche la label
            otherNode = agnode(graphOut, to_string(statusSet.size()-1).c_str(), TRUE);  
            agsafeset(otherNode, "label", to_string(statusSet.size()-1).c_str(), "");    

            //Se contiene nodi finali allora è uno stato finale
            if(isFinal(graphIn, statusSet[statusSet.size()-1]))
              agset(otherNode, "shape", "doublecircle");     
          }
          else
          {
            //Se esiste già
            //Prendo l'altro nodo per creare un arco
            otherNode = agnode(graphOut, to_string(got->second).c_str(), FALSE);
          }
          
          //Creo l'arco
          char tmpStr[2] = {alpha, 0};
          Agedge_t *nowEdge = agedge(graphOut, nowNode, otherNode, tmpStr, TRUE);
          agsafeset(nowEdge, "label", tmpStr, "");
        }
      }
    }
  }
  
  return;
}

bool isFinal(Agraph_t *graph, const bitset<MAX_NODE> set)
{
  Agnode_t *node;
  char nodeId[5];

  int nNodi = set.count();

  for(short i = 0; i < MAX_NODE && nNodi > 0; ++i)
    if(set[i])
    {
      --nNodi;

      //Prendo l'id del nodo
      sprintf(nodeId, "%d", i);
      //Prendo il nodo
      node = agnode(graph, nodeId, FALSE);

      if(!strcmp(agget(node, "shape"), "doublecircle"))
        return true;
    }

  return false;
}

bitset<MAX_NODE> epsClosure(Agraph_t *graph, const bitset<MAX_NODE> startSet)
{
  //Inizializzo l'insieme di ritorno con l'insieme di partenza
  bitset<MAX_NODE> endSet = startSet;

  //Creo una coda con gli elementi dello startSet
  queue<unsigned short> nodeQueue;
  //Riempio la coda con il riferimento agli elementi dello startSet
  for (short i = 0; i < MAX_NODE; ++i)
    if(startSet[i])
      nodeQueue.push(i);

  char nodeId[6];
  short linkNodeId;
  Agnode_t *node;
  Agedge_t *edge;

  while (!nodeQueue.empty())
  {
    //Prendo l'id del nodo dalla coda
    sprintf(nodeId, "%d", nodeQueue.front());
    //Elimino l'id del nodo dalla coda
    nodeQueue.pop();
    //Prendo il nodo
    node = agnode(graph, nodeId, FALSE);

    //Per tutti gli archi di del nodo preso
    for (edge = agfstout(graph, node); edge; edge = agnxtout(graph,edge))
    {
      linkNodeId = atoi(agnameof(edge->node));
      //Se il nodo non è già stato visitato
      // e se esiste una epsilon transizione da questo nodo
      if(!endSet[linkNodeId] && !strcmp(agget(edge, "label"), "eps"))
      {
        //Aggiugno l'id del nodo alla coda
        nodeQueue.push(linkNodeId);
        //Inserisco l'id del nodo nell'endset
        endSet[linkNodeId] = 1;
      }
    }
  }

  return endSet;
}

bitset<MAX_NODE> alphaClosure(Agraph_t *graph, const bitset<MAX_NODE> startSet, char transition)
{
  bitset<MAX_NODE> endSet;
  char nodeId[5];
  Agnode_t *node;
  Agedge_t *edge;

  //Per ogni arco di ogni nodo nell'insieme di partenza
  for(short i = 0; i < MAX_NODE; ++i)
    if(startSet[i])
    {
      //Converto l'id del nodo
      sprintf(nodeId, "%d", i);
      //Prendo il nodo
      node = agnode(graph, nodeId, FALSE);

      for (edge = agfstout(graph, node); edge; edge = agnxtout(graph,edge)) 
        //se esiste una alpha transizione corretta da questo nodo
        if(agget(edge, "label")[0] == transition)
        {
          //Inserisco il nodo nell'insieme di uscita
          endSet[atoi(agnameof(edge->node))] = 1;
          break;//Poichè c'è solo una transizione corretta
        }
    }
  return endSet;
}