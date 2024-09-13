#include "./view_maintenance_visitor.h"
#include "antlr4-runtime.h"
#include "generated/LcypherLexer.h"
#include "generated/LcypherParser.h"
using namespace parser;
using namespace antlr4;
#define NODE_INF std::tuple<std::string,std::string,std::string,bool>

void RewriteForVertex(std::string view_query,std::string node_label,std::string primary_key,std::string primary_value,bool value_is_string,bool is_create){
    ANTLRInputStream input(view_query);
    LcypherLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    // std::cout <<"parser s1"<<std::endl; // de
    LcypherParser parser(&tokens);
    ViewMaintenance visitor(parser.oC_Cypher(),node_label,primary_key,primary_value,value_is_string,is_create);
        for(auto query: visitor.GetRewriteQueries()){
            std::cout<<"RewriteForVertex query: "<<query<<std::endl;
        }
    }

void RewriteForEdge(std::string view_query,std::string edge_label,int edge_id,NODE_INF src,NODE_INF dst,bool is_create){
    ANTLRInputStream input(view_query);
    LcypherLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    // std::cout <<"parser s1"<<std::endl; // de
    LcypherParser parser(&tokens);
    ViewMaintenance visitor(parser.oC_Cypher(),edge_label,edge_id,src,dst,is_create);
    std::vector<std::string> queries=visitor.GetRewriteQueries();
        for(auto query: visitor.GetRewriteQueries()){
            std::cout<<"RewriteForEdge query: "<<query<<std::endl;
        }
}


int main(){
    std::string script="match (n)-[r*2..4]->(t)-[]->(m) WITH n,m CREATE (n)-[r:test]->(m)";
    // std::string script;
    // std::cin>>script;
    NODE_INF src("person","id","1",false);
    NODE_INF dst("movie","id","2",false);
    RewriteForEdge(script,"acted_in",1,src,dst,false);
    RewriteForVertex(script,"keyword","id","1",false,false);
}