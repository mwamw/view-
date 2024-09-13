# 视图相关的论文
## 图数据库
- [Implementation Strategies for Views Over Property Graphs, SIGMOD, 2024](https://dl.acm.org/doi/pdf/10.1145/3654949)
    * 第一个在图上使用视图进行查询优化的工作
    * graph views->TGD->Datalog->SQL，在关系型数据库中用SSR(subgraph substitution relation)来表示视图
    * 对更新机制没有过多叙述，定义了well-behavedness的视图
    * 实验在关系型数据库和图数据库上同时进行，结论是SSR可以帮助关系型数据库在图数据库甚至图视图中取得很大的竞争优势(在合适的索引下)
    * 目前无开源代码(https://github.com/PennGraphDB/pg-view)
- [Kaskade: Graph views for efficient graph analytics, ICDE, 2020](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=9101351)
    * 第一个在图上使用视图进行查询优化的工作
    * 根据查询负载确定需要物化的视图，然后对于实际的查询应用视图来响应
    * 没有提到更新机制，主要针对查询负载进行优化
    * 无开源代码
- [Exploring Lightweight Materialized View Designs for Transactional Graph Processing(未发布)](https://xxx)
    * 在图上做OLTP，实现查询优化，使用虚拟属性和路径折叠两种物化视图的构建方法，实现常量时间的维护代价
    * 有更新机制，针对于特定的视图，种类较少？
    * 无开源代码
- [Partial Update: Efficient Materialized View Maintenance in a Distributed Graph Database, ICDE, 2018](https://ieeexplore.ieee.org/document/8509387)
    * 在分布式图数据库上实现视图的增量维护
    * 有更新机制，视图只有一跳或两跳，种类过少
    * 无开源代码
- [Graphsurge: Graph Analytics on View Collections Using Differential Computation, SIGMOD, 2021](https://dl.acm.org/doi/pdf/10.1145/3448016.3452837)
    * 有开源代码
    * 视图定义比较简单，在图上或者别的视图上定义，只包含一个Where子句，输出符合的边集合作为视图
- [Exploring views in graph database system](https://escholarship.mcgill.ca/concern/theses/z890s042x)
    * 在Neo4j上实现物化视图机制的解决方案，其在Cypher语句层面扩展关键字来引入视图。并且通过作用域关键字将视图和图数据库结合来响应查询
    * 缺少对于图结构视图的支持
    * 并且视图的更新机制不够完善（使用全量更新的方式）

## 关系型数据库
