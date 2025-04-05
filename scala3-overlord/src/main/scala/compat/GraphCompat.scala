package compat

import org.jgrapht.graph.*
import org.jgrapht.nio.dot.*
import org.jgrapht.nio.json.*
import org.jgrapht.traverse.*

import scala.collection.mutable
import scala.jdk.CollectionConverters.*

/**
 * Compatibility layer for scala-graph using JGraphT.
 * This provides a bridge between the original scala-graph API used in Scala 2
 * and the JGraphT library used in Scala 3.
 */
object GraphCompat:
  // A simplified Graph implementation that mimics some of the scala-graph API
  class Graph[N, E]:
    private val graph = new DirectedMultigraph[N, LabeledEdge[N, E]](classOf[LabeledEdge[N, E]])
    
    // Add a node to the graph
    def +(node: N): this.type =
      graph.addVertex(node)
      this
    
    // Add multiple nodes to the graph
    def ++(nodes: Iterable[N]): this.type =
      nodes.foreach(graph.addVertex)
      this
    
    // Add an edge between two nodes
    def +=(from: N, to: N, label: E): this.type =
      if !graph.containsVertex(from) then graph.addVertex(from)
      if !graph.containsVertex(to) then graph.addVertex(to)
      graph.addEdge(from, to, new LabeledEdge(from, to, label))
      this
    
    // Get all nodes in the graph
    def nodes: Set[N] = graph.vertexSet().asScala.toSet
    
    // Get all edges from a node
    def outEdges(node: N): Set[(N, N, E)] =
      graph.outgoingEdgesOf(node).asScala.map(e => 
        (e.getSource, e.getTarget, e.getLabel)
      ).toSet
    
    // Get all edges to a node
    def inEdges(node: N): Set[(N, N, E)] =
      graph.incomingEdgesOf(node).asScala.map(e => 
        (e.getSource, e.getTarget, e.getLabel)
      ).toSet
    
    // Get a DOT representation of the graph
    def toDot: String =
      val exporter = new DOTExporter[N, LabeledEdge[N, E]]()
      val writer = new java.io.StringWriter()
      exporter.exportGraph(graph, writer)
      writer.toString
      
    // Get a JSON representation of the graph
    def toJson: String =
      val exporter = new JSONExporter[N, LabeledEdge[N, E]]()
      val writer = new java.io.StringWriter()
      exporter.exportGraph(graph, writer)
      writer.toString
    
    // Traverse the graph in depth-first order
    def dfs(start: N): Iterable[N] =
      val iterator = new DepthFirstIterator(graph, start)
      iterator.asScala.toList
    
    // Traverse the graph in breadth-first order
    def bfs(start: N): Iterable[N] =
      val iterator = new BreadthFirstIterator(graph, start)
      iterator.asScala.toList
    
    // Get the internal JGraphT graph
    def raw: DirectedMultigraph[N, LabeledEdge[N, E]] = graph

  // Custom edge class to maintain edge labels
  class LabeledEdge[N, E](source: N, target: N, label: E) extends DefaultEdge:
    override def getSource: N = source
    override def getTarget: N = target
    def getLabel: E = label
    
    override def toString: String = s"$source -> $target : $label"

  // Factory method to create a new graph
  def graph[N, E](): Graph[N, E] = new Graph[N, E]()