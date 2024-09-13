# 之前的视图维护
## 例子
之前的视图维护是分为两步：
* 1.可变长展开：
    * 比如match (n)-[r*1..3]->(m)重写成(3-1+1)=3条语句：
        * match (n)-[r1]->(m)
        * match (n)-[r1]->()-[r2]->(m)
        * match (n)-[r1]->()-[r2]->()-[r3]->(m)
* 2.对每条不含可变长的语句重写维护语句:
    * 比如从person到keyword的两跳视图：match (n:person)-[]->(a)-[]->(m:keyword) WITH n,m CREATE (n)-[r:test]->(m)，当删除某个点比如keyword{id:$id}时，要对这个点影响的视图边进行删除，那么其实有两个可能，这个点在a点或者在m点(n点由于标签原因不能匹配)：
        * 情况1(在m点)： match (n:person)-[]->()-[]->(m:keyword{id:$id}) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
        * 情况2(在a点)：match (n:person)-[]->(a:keyword{id:$id})-[]->(m:keyword) with n,m match (n)-[r:test  NoDupEdge]->(m) delete r
## 具体说明
* 第一步的代码在 https://github.com/S4Plus/tugraph-db/blob/master/src/cypher/parser/varlen_unfold_visitor.h ，这部分比较简单，最终用GetRewriteQueries()函数返回展开后的语句集合
* 第二步的代码在 https://github.com/S4Plus/tugraph-db/blob/master/src/cypher/parser/view_maintenance_visitor.h ，这部分首先分两种情况：
    * 1.创建点或边
        * a.创建点其实并不影响，因为创建的点没有和任何边相连，是个孤立的点，所以直接没有处理
        * b.创建边的话，就是把 (src)-[r]->(dst)这个三元组不断去匹配路径，需要src，r，dst同时匹配，每次匹配成功则创建一条重写语句，如果添加某条边r((:person{id:1})-[r:write]->(:movie{id:2}),id(r)=3)，则重写语句只有一条，如下：
            * match (n:person{id:1})-[r:write]->(:movie{id:2})-[]->(m:keyword) where id(r)=3 with n,m create (n)-[r:test]->(m)
    * 2.删除点或边
        * a.删除点的话和之前的例子一样，比边的匹配简单一些，只需要匹配到某个点就可以进行重写
        * b.删除边的话和创建边类似，其实插入和删除的区别并不大，只是with语句之后的区别，如果删除某条边r((:person{id:1})-[r:write]->(:movie{id:2}),id(r)=3)，则重写语句只有一条，如下：
            * match (n:person{id:1})-[r:write]->(:movie{id:2})-[]->(m:keyword) where id(r)=3 with n,m match (n)-[r:test  NoDupEdge]->(m) delete r

## 问题
先不说效率上的问题，虽然TuGraph对于无上界的可变长的处理是人为设置了上界128，但是我们在处理时不应该认为它有上界，对于无上界的可变长边根本无法展开，所以目前考虑并不完全展开可变长边。

# 新的方式
依然分为对点和边的处理，以及插入删除的区别，不过插入删除的部分不用动，只需要改变前面的Match部分的重写
## 例子
### 删除点
* 并不分成两步，对于可变长边有特殊的处理，比如说Match (n)-[r*2..]->(m)这样的视图，我们删除某个点k{id:$id}时，k点要么是n,m，要么是这个可变长边里面的点，所以可以把可变长边分成两个部分，一部分是n到k，一部分是k到m：
    * Match (n)-[\*0]->(k)-[\*2..]->(m)
    * Match (n)-[\*1]->(k)-[\*1..]->(m)
    * Match (n)-[\*2..]->(k)-[\*0..]->(m)
* 就是说n到k如果有j1(j1<2)跳，那么k到m就是2-j1到无穷跳，如果n到k的跳数大于等于2，那么k到m就是0到无穷跳
* 那么对于match (n)-[r*2..]->(t)-[]->(m) WITH n,m CREATE (n)-[r:test]->(m)这样的视图，删除某个点k:keyword{id:$id}时，重写语句就是：
    * match (n)-[\*0]->(k:keyword{id:$id})-[\*2..]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*1]->(k)-[\*1..]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*2..]->(k)-[\*0..]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n:keyword{id:$id})-[r*2..]->(t)-[]->(m) WITH n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[r*2..]->(t:keyword{id:$id})-[]->(m) WITH n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[r*2..]->(t)-[]->(m:keyword{id:$id}) WITH n,m match (n)-[r:test NoDupEdge]->(m) delete r
* 前三条语句就是删除的点在可变长边中的情况，后三条语句就是匹配正常出现的点，其实匹配n点或m点在第一条语句和第三条语句中已经出现了这种情况
### 创建或删除边
* 同样将可变长边分成两个部分，说Match (n)-[r*2..]->(m)这样的视图，删除点r，那么将(src)-[r]->(dst)放在可变长边中：
    * Match (n)-[\*0]->(src)-[r]->(dst)-[\*1..]->(m)
    * Match (n)-[\*1..]->(src)-[r]->(dst)-[\*0..]->(m)
* 对于最少k跳的无上界可变长有k种重写情况，那么对于match (n)-[r*2..]->(t)-[]->(m) WITH n,m CREATE (n)-[r:test]->(m)这样的视图，删除某个边r((:person{id:1})-[r:write]->(:movie{id:2}),id(r)=3)时，重写语句就是：
    * match (n)-[\*0]->(:person{id:1})-[r:write]->(:movie{id:2})-[\*1..]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*1..]->(:person{id:1})-[r:write]->(:movie{id:2})-[\*0..]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*2..]->(t:person{id:1})-[r:write]->(m:movie{id:2}) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
* 前两条是在可变长中的情况，分别表示删除的边在第一跳或者是第二跳及以后，最后一条是之前出现的情况
    match (n)-[r*2..4]->(t)-[]->(m):
    * match (n)-[\*0]->(:person{id:1})-[r:write]->(:movie{id:2})-[\*1..3]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*1]->(:person{id:1})-[r:write]->(:movie{id:2})-[\*0..2]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*2]->(:person{id:1})-[r:write]->(:movie{id:2})-[\*0..1]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r
    * match (n)-[\*3]->(:person{id:1})-[r:write]->(:movie{id:2})-[\*0]->(t)-[]->(m) with n,m match (n)-[r:test NoDupEdge]->(m) delete r    

### 其他的说明
* 对于有上界的可变长其实类似，只不过对k到m添加了上界，比如说Match (n)-[r*2..4]->(m)这样的视图，我们删除某个点k{id:$id}时，一部分是n到k，一部分是k到m：
    * Match (n)-[\*0]->(k)-[\*2..4]->(m)
    * Match (n)-[\*1]->(k)-[\*1..3]->(m)
    * Match (n)-[\*2]->(k)-[\*0..2]->(m)
    * Match (n)-[\*3]->(k)-[\*0..1]->(m)
    * Match (n)-[\*4]->(k)-[\*0]->(m)
* 上界是n的话就有n条

## **需要更改的部分**
* 所以现在需要更改varlen_unfold_visitor.h和view_maintenance_visitor.h，可以把varlen_unfold_visitor.h直接删掉，实现上述的重写，因为view_maintenance_visitor.h没有过多的注释，所以对TuGraph定义的数据结构或者原来的实现有问题的话可以直接问我
* 为了方便调试，我把重写的部分提取出来，放在 http://222.195.92.204:1480/graph/graph-views/-/tree/master/parser?ref_type=heads ，可以现在这个上面改，然后测试
* 关于更新维护这部分有任何好的想法都可以和我讨论