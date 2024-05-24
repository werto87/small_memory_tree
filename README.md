# small_memory_tree
## Motivation and Goals
small_memory_tree saves data plus hierarchy information of a tree. It tries to **save memory** compared to other libraries who use a vector plus a pointer to the parent as a node(vecPlusPointerToParentNode). Even an empty vector needs 24 Bytes of memory (3 pointer with 8 Bytes on a 64 bit cpu) plus the pointer to the parent node which is again 8 bytes this results in 32 bytes of memory (overhead) + memory needed for the value we want to save (payload). If the payload is one byte and the overhead is 32 bytes this means from 33 bytes only one byte is actual useful. small_memory_tree saves the whole data of a tree in a vector (where the ValueType is the ValueType of tree), the hierarchy information in another vector (where the ValueType is bool) and a number called max children (integral type). This also means that small_memory_tree can be **saved to disk relative easily**.

## How small_memory_tree saves a Tree in Memory
small_memory_tree saves a tree in the struct SmallMemoryTreeData. SmallMemoryTreeData contains 3 member variables:
```cpp
  MaxChildrenType maxChildren{};
  std::vector<ValueType> data{};
  std::vector<bool> hierarchy{};
```
### MaxChildrenType maxChildren{};
#### MaxChildrenType
MaxChildrenType is an integral type. The type can be selected by the user or the default type uint64_t will be used.
#### What is maxChildren and how it gets calculated
maxChildren gets calculated by counting the children per node and saving the highest value.
### std::vector<ValueType> data{};
#### ValueType
ValueType is the type of the values from the tree (MyCoolTree<int> here int is the ValueType).
#### What is data and how it gets calculated
data contains the values from tree in breadth first order.
### std::vector<bool> hierarchy{};
#### What is hierarchy and how it gets calculated
hierarchy contains the hierarchical information of the tree. Together with maxChildren it is possible to tell how the nodes are related.
To calculate hierarchy we add true to hierarchy for the root element (Todo this can be saved. If we always add true at the start we can assume it is there and do not have to add it) and than iterate breadth first over the tree and ask every node how many children it has. For everyChild we add true to the vector. If a node has less children than maxChildren we fill the missing values with false.

### Example how SmallMemoryTreeData gets created
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
#### maxChildren
maxChildren gets calculated by asking every node how much children it has and saving the highest value in maxChildren.
In this example maxChildren equals 2.
#### data
Iterate over the tree in breath first an save the values in data. data= [0,1,2,3,4]
#### hierarchy
- Add true to hierarchy for the root element. hierarchy contains [true]
- Iterate breadth first over the tree and ask the node how many children it has.
- Add true to hierarchy for every child. Add false for every child missing child
- root has two children and maxChildren is two so hierarchy contains [true,true,true] after visiting the root node
- the node 1 has 1 child and maxChildren is two so hierarchy contains [true,true,true,true,false] after visiting the node 1
- the node 2 has 0 children and maxChildren is two so hierarchy contains [true,true,true,true,false,false,false] after visiting the node 2
- the node 3 has 1 child and maxChildren is two so hierarchy contains [true,true,true,true,false,false,false,true,false] after visiting the node 3
- the node 4 has 0 children and maxChildren is two so hierarchy contains [true,true,true,true,false,false,false,true,false,false,false] after visiting the node 4

## Memory Consumption
### In which case does small memory tree actually save memory compared to vecPlusPointerToParentNode
If using small memory tree saves you memory or not depends heavily on the max children. Figure 1 shows an overview of how much heap memory small memory tree uses compared to a vecPlusPointerToParentNode tree (stlplus::ntree). The values are measured using the project in this [repository](https://github.com/werto87/small_memory_tree_memory_measurement). The structure of the tree is worst case for small memory tree. Just a root with children who are leafs. Around a max children count of 580 small memory tree
needs more memory than stlplus tree.

![image](https://github.com/werto87/small_memory_tree/assets/46565959/09fbc0a0-b5f1-492f-9233-935f8fcaca78)
Figure 1: Heap Memory Consumption 'stlplus tree' vs 'small memory tree'  

Because it is hard to see the Byte usage for small values of max children count in Figure 1 Figure 2 shows a zoomed version of Figure 1.

![image](https://github.com/werto87/small_memory_tree/assets/46565959/caea0eea-2ca0-4d55-b014-424d58789613)
Figure 2: Zoom on small values for max children Heap Memory Consumption


## Usage Example with [st_tree](https://github.com/erikerlandson/st_tree)
As always for examples look in the tests for example in test/stTree.cxx
### Use Case store tree and load it from the database
```cpp
#include <cassert>
#include <small_memory_tree/stTree.hxx>
int
main ()
{
  using namespace small_memory_tree;
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto dataToDatabase = SmallMemoryTreeData<int>{ StTreeAdapter{ tree } };
  // save result.data; result.hierarchy; result.maxChildren into the database (you have to write this code by yourself. It is currently not in the scope of small memory tree.);
  // a couple lines of code later...
  // load data from database (you have to write this code by yourself. It is currently not in the scope of small memory tree.)
  auto maxChildrenFromDatabase = uint64_t{ 2 };
  auto treeDataFromDatabase = std::vector<int>{ 0, 1, 2 };
  auto treeHierarchyFromDatabase = std::vector<bool>{ true, true, true, false, false, false, false };
  // create SmallMemoryTreeData from the data from the database
  auto dataFromDatabase = SmallMemoryTreeData<int>{ maxChildrenFromDatabase, std::move (treeHierarchyFromDatabase), std::move (treeDataFromDatabase) };
  // create SmallMemoryTree from SmallMemoryTreeData
  auto smallMemoryTreeFromDatabase = SmallMemoryTree<int>{ std::move (dataToDatabase) };
  // create a st_tree from SmallMemoryTree
  auto treeFromDatabase = generateStTree (smallMemoryTreeFromDatabase);
  assert (treeFromDatabase == tree);
  return 0;
}
```

### Traverse small memory tree (getChildrenByPath)
```cpp
#include <cassert>
#include <small_memory_tree/stTree.hxx>
int
main ()
{
  using namespace small_memory_tree;
  // create and fill a tree
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  // from the tree create a small memory tree
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
  // use childrenByPath to get children of the root
  auto myChildren = childrenByPath (smt, std::vector<int>{ 0 });
  assert (myChildren.has_value ());
  assert (myChildren.value ().at (0) == 1);
  assert (myChildren.value ().at (1) == 2);
  // use childrenByPath to get children of root's child with the value 1
  auto myChildrenOfRootChildWithTheValue1 = childrenByPath (smt, std::vector<int>{ 0, 1 });
  assert (myChildrenOfRootChildWithTheValue1.has_value ());
  assert (myChildrenOfRootChildWithTheValue1.value ().at (0) == 3);
  return 0;
}
```

## Build
### Requirements
- Compiler with support c++20 support (ranges and concepts)
- CMake
- The following c++ libraries are required:
  - [boost](https://github.com/boostorg/boost)/1.84.0 
  - [confu_algorithm](https://github.com/werto87/confu_algorithm)/1.0.1

- The following c++ libraries are optional:
  - [stlplus](https://github.com/werto87/stlplus3)/3.16.0  Pass WITH_STLPLUS_TREE to cmake  
  - [st_tree](https://github.com/erikerlandson/st_tree)/1.2.2  Pass WITH_ST_TREE to cmake 
  - [catch2](https://github.com/catchorg/Catch2)/2.13.7 Pass WITH_TESTS to cmake  

### The recommended way to build small memory tree is using the conan package manager
#### As someone who wants to contribute to small memory tree development
1. Check out small memory tree
2. Use conan remote to add https://modern-durak.com/artifactory/conan-local/
3. Run conan install
4. Configure CMake with:
    - BUILD_TESTS true
    - WITH_ST_TREE true
    - WITH_STLPLUS_TREE true
    - CMAKE_TOOLCHAIN_FILE conan_toolchain.cmake
5. Build the project using CMake
6. Run the tests target with build/test/_test

#### As  someone who wants to use small memory tree as a library
1. Use conan remote to add https://modern-durak.com/artifactory/conan-local/
2. Add small_memory_tree/SomeVersion as a requirement to your conanfile.py
3. If you want to use st_tree::tree or stlplus::ntree set the option with_st_tree or with_stlplus_tree to true
4. Add small_memory_tree to your CMake