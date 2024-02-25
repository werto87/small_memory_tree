# small_memory_tree

small_memory_tree saves data plus hierarchy information of a tree. It tries to save memory compared to other libraries who use a vector plus a pointer to the parent as a node(vecPlusPointerToParentNode). Even an empty vector needs 24 Bytes of memory (3 pointer a 8 bytes on a 64 bit cpu) plus the pointer to the parent node which is again 8 bytes this results in 32 bytes of memory (overhead) + memory needed for the value we want to save (payload). If the payload is one byte and the overhead is 32 bytes this means from 33 bytes only one byte is actual useful. small_memory_tree saves the whole tree in a vector. To preserve the hierarchy information small_memory_tree adds special marker to the vector (overhead).


## How small_memory_tree saves data plus hierarchy information 

Lets say we want to save a tree with 5 nodes who's values fit in unsigned char for example like this:
```
        0
       / \
      1   2
     /
    3
   /
  4
```
### add marks

Every node has to have the same amount of children as the node with the most children (In this example 2 nodes) Expect if the node is marked as empty. To mark a node as empty small_memory_tree uses a special value set by the user with the same datatype as the vector elements. For example unsigned char max which is 255. This results in a tree like this:

```
          0
        /   \
       1      2
      /\     / \
     3 255 255 255
    /  \
   4   255
  / \
255 255
```
### save into vector

small_memory_tree saves the values in a vector. Going left to right top to bottom.
This results in a vector like this: [0,1,2,3,255,255,255,4,255,255,255]. Also small_memory_tree adds the max children as an extra element which is in this example 2. So the resulting vector is:[0,1,2,3,255,255,255,4,255,255,255,2]. 


## Comparison  'vecPlusPointerToParentNode' vs 'small_memory_tree'
vecPlusPointerToParentNode case every node costs 33 bytes. There are 5 nodes. The total memory is 165 bytes. For small_memory_tree its a vector with 12 elements every element is one byte plus the 24 bytes from the vector results in 36 bytes.


## example in which small_memory_tree uses more memory compared to vecPlusPointerToParentNode
Lets say we want to save a tree with 1001 nodes who's values fit in unsigned char for example like this:
```
L0         0
          / \\\\\\ ...
L1       1000 children
```

max children for this tree is 1000 so small memory tree would add markers until every node has 1000 children besides nodes which are marked as leaf (value 255 in this example). So the tree would look like this:

```
Level 0            0
                  / \\\\\\ ...
L1               1000 nodes with a value different from 255
                / \\\\\\ ...
L2             1000 times 1000 nodes to mark parent nodes as leaf
```
```
In total this is:  
L0                                                    1 node  
L1                                                1'000 nodes  
L2 1000 nodes * 1000 nodes(marker for leaf)   1'000'000 nodes  
--------------------------------------------------------------  
total nodes                                   1'001'001 nodes  
```
1'001'001 times 1 byte plus the 24 bytes from the vector pointers equals 1'001'025 bytes for small memory tree and 1004 times 33 bytes equals 33132 bytes for vecPlusPointerToParentNode.


## example in which small_memory_tree uses roughly the same memory compared to vecPlusPointerToParentNode
Lets say we want to save a tree with 34 nodes who's values fit in unsigned char for example like this:
```
L0         0
          / \\\\\\ ...
L1       33 children
```

max children for this tree is 33 so small memory tree would add markers until every node has 33 children besides nodes which are marked as leaf (value 255 in this example). So the tree would look like this:

```
Level 0            0
                  / \\\\\\ ...
L1               33 nodes with a value different from 255
                / \\\\\\ ...
L2             33 times 33 nodes to mark parent nodes as leaf
```
```
In total this is:  
L0                                                    1 node  
L1                                                   33 nodes  
L2 33 nodes * 33 nodes(marker for leaf)            1089 nodes  
--------------------------------------------------------------  
total nodes                                        1123 nodes  
```
1123 times 1 byte plus the 24 bytes from the vector pointers equals 1147 bytes for small memory tree and 34 times 33 bytes equals 1122 bytes for vecPlusPointerToParentNode.


## idea to improve small_memory_tree memory efficiency
create a small memory tree using bool and save its hierarchy information. create another vector with the data. this could save a ton of memory if the tree has a lot of children and the data type of the nodes is for example 8 bytes long and not 1 byte.


## Requirements



<!-- TODO REWRITE -->
## How to use

```cpp
        0
       / \
      1   2
     /
    3
   /
  4

auto tree = st_tree::tree<uint8_t>{}; // create a tree
tree.insert (0); // fill it
tree.root ().insert (1);
tree.root ().insert (2);
tree.root ()[0].insert (3);
tree.root ()[0][0].insert (4);
auto compressedTree = treeToVector (tree, uint8_t{ 255 }); // creates a vector with the data from tree using 255 as marker for empty and 254 for child
// 'compressedTree' can be queried for data using for example the childrenByPath function TODO write example
// note in this case vector is a byte vector which can be easily stored in a database.
auto decompressedTree = vectorToTree (compressedTree); // if the data of compressedTree needs to be altered it can be decompressed.
assert (decompressedTree == tree);                     // compares 'decompressedTree' with 'tree'
```

## Difference between st_tree and small_memory_tree

st_tree goals from [st_tree git readme](https://github.com/erikerlandson/st_tree)
> The st_tree library allows the programmer to easily declare and manipulate data in a tree

The goals of small_memory_tree are:

- Saving memory compared to a st_tree (depending on the structure of your tree. Read " Do not use small_memory_tree if"
  section").
- Saving a tree to disk and restore it.
- Lookup of values in the tree using the childrenByPath.

Common tree functions are not supported on a compressed tree:  
`
Note you can decompress the compressed tree into a st_tree and perform all common tree functions and then compress it
again.
`

## Do not use small_memory_tree if

- Your data on the nodes does not allow to set two marks. You should have 2 free values which small_memory_tree can use
  as marks (for example uint8_t 254 and 255)
- You have a st_tree with a lot of nodes with a low amount of children and a few nodes with a lot of children. In this
  case small_memory_tree could get bigger than st_tree
- Your data type is small for example uint8_t, and your tree is wide.


