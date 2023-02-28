#include "networkit/auxiliary/Enforce.hpp"
#include "networkit/auxiliary/StringTools.hpp"
#include "networkit/io/GXLGraphReader.hpp"

namespace NetworKit {

  Graph GXLGraphReader::read(const std::string &path) {

    std::ifstream graphFile(path);
    try {
      Aux::enforceOpened(graphFile);
    } catch (std::runtime_error &e) {
      throw std::runtime_error("could not open file " + path);
    }
    std::string line;

    auto syntaxCheck = [](const std::vector<std::string>& tokens) {
      std::stack<std::string> openTags;
      std::string cur;
      for (auto token: tokens) {
        if (token[0] != '/') {
          auto end = token.find(' ');
          cur = token.substr(0, end);
          if (cur == "node" || cur == "edge" || cur == "graph" || cur == "attr" || cur == "string" || cur == "int" ||
              cur == "Float" || cur == "Double") {
            openTags.push(cur);
          }
        } else {
          if (openTags.empty())
            throw std::runtime_error("encountered opening tag without closing tag" + token);
          if (openTags.top() != token.substr(1))
            throw std::runtime_error(
                    "encountered closing tag for " + token.substr(1) + " but expected closing tag for " +
                    openTags.top());
          openTags.pop();
        }
      }
    };

    /**
     * returns vector of tokens (contents between < and >)
     */
    auto tokenize = [](std::string &line, NetworKit::index i) {
      std::vector<std::string> tokens;
      NetworKit::index start = i;
      NetworKit::index end = i;
      while (end < line.size()) {
        if (line[end] == '<') {
          ++end;
          start = end;
          while (line[end] != '>') {
            if (line[end] == '<') {
              throw std::runtime_error("encountered another < before closing >");
            } else if (end == line.size() - 1) {
              throw std::runtime_error("encountered end of line before closing >");
            }
            ++end;
          }
          tokens.push_back(line.substr(start, end - start));
          ++end;
        } else if (line[end] != '<' && end < line.size()) {
          start = end;
          while (line[end] != '<') {
            if (line[end] == '>')
              throw std::runtime_error("encountered > before opening <");
            else if (end == line.size() - 1)
              throw std::runtime_error("encountered end of line before opening <");
            ++end;
          }
          tokens.push_back(line.substr(start, end - start));
        }
      }
      return tokens;
    };

    /**
     * assumes attributed nodes
     * supports lines of the form: <node id="XYZ"><attr name="XYZ"><type>XYZ</type></attr></node>
     * matching the format found in e.g. Mutagenicity dataset
     * (multiple attributes per node as in CMU, or Proteins is not supported)
     * double as attribute get casted to float
     *
     *
     */
    auto parseNode = [](Graph &G, std::vector<std::string> tokens) {
      NetworKit::index i = 0;
      std::string attrName;
      std::string attrType = tokens[2];
      std::string nodeAttr;

      if (tokens[1].find("attr") == std::string::npos) {
        throw std::runtime_error("line containing node element does not have attr element");
      }

      auto startName = tokens[1].find('"', 0);
      auto endName = tokens[1].find('"', startName + 1);
      attrName = tokens[1].substr(startName + 1, endName - startName - 1);

      auto node = G.addNode();

      if (attrType == "int" || attrType == "Int" || attrType == "integer" || attrType == "Integer") {
        auto intAttr = G.attachNodeIntAttribute(attrName);
        intAttr.set(node, std::stoi(tokens[3]));
      } else if (attrType == "string" || attrType == "String" || attrType == "str" || attrType == "Str") {
        auto strAttr = G.attachNodeStringAttribute(attrName);
        strAttr[node] = std::string(tokens[3]);
      } else if (attrType == "float" || attrType == "Float" || attrType == "double" || attrType == "Double") {
        auto doubleAttr = G.attachNodeDoubleAttribute(attrName);
        doubleAttr[node] = std::stod(tokens[3]);
      } else {
        throw std::runtime_error("Node attribute not int, float, double or string ");
      }
    };

    auto parseEdge = [](Graph &G, std::vector<std::string> tokens) {
      int u;
      int v;
      std::string attrType = tokens[2];
      double attrValue = std::stod(tokens[3]);

      auto startNode1 = tokens[0].find('"', 0);
      auto endNode1 = tokens[0].find('"', startNode1 + 1);
      auto startNode2 = tokens[0].find('"', endNode1 + 1);
      auto endNode2 = tokens[0].find('"', startNode2 + 1);
      u = std::stoi(tokens[0].substr(startNode1 + 1, endNode1 - startNode1 - 1));
      v = std::stoi(tokens[0].substr(startNode2 + 1, endNode2 - startNode2 - 1));

      if (attrType == "int" || attrType == "Int" || attrType == "integer" || attrType == "Integer" ||
          attrType == "float" || attrType == "Float" || attrType == "double" || attrType == "Double") {
        G.addEdge(u, v, attrValue);
      } else {
        throw std::runtime_error("Edge attribute not int, float or double");
      }
    };

    auto parseHead = [](std::ifstream &graphFile, std::string &line) {
      bool directed = false;
      bool edgeids = false;

      std::getline(graphFile, line);
      if (line.find("xml") == std::string::npos) {
        throw std::runtime_error("expected xml tag, file format not .gxl");
      }
      std::getline(graphFile, line);
      if (line.find("gxl") == std::string::npos) {
        throw std::runtime_error("expected gxl tag, file format not .gxl");
      }
      std::getline(graphFile, line);
      if (line.find("graph") == std::string::npos) {
        throw std::runtime_error("expected graph tag, file format not .gxl");
      }
      if (line.find("edgemode") != std::string::npos) {
        if (line.find("directed") != std::string::npos) {
          directed = true;
        }
      }
      auto pos = line.find("edgeids=");
      if (line.find("edgeids") != std::string::npos) {
        if (line.find("true") != pos + 9 && line.find("true") != std::string::npos) {
          edgeids = true;
        }
      }
      return Graph(0, true, directed, edgeids);
    };

    /**
     * maybe implement converter that fits each gxl element into one line (e.g. CMU housing data set splits these into multiple lines)
     */
    auto parseFile = [&]() -> Graph {
      Graph G = parseHead(graphFile, line);

      while (getline(graphFile, line)) {
        if (line.find("node") != std::string::npos) {
          auto tokens = tokenize(line, 0);
          syntaxCheck(tokens);
          parseNode(G, tokens);
        } else if (line.find("edge") != std::string::npos) {
          auto tokens = tokenize(line, 0);
          syntaxCheck(tokens);
          parseEdge(G, tokens);
        }
      }
      return G;

    };

    Graph G = parseFile();
    G.shrinkToFit();
    return G;
  }

} /* namespace NetworKit */