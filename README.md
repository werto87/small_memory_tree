# small_memory_tree
## Motivation and Goals
small_memory_tree saves data plus hierarchy information of a tree. It tries to **save memory** compared to other libraries who use a vector plus a pointer to the parent as a node(vecPlusPointerToParentNode). Even an empty vector needs 24 Bytes of memory (3 pointer with 8 Bytes on a 64 bit cpu) plus the pointer to the parent node which is again 8 bytes this results in 32 bytes of memory (overhead) + memory needed for the value we want to save (payload). If the payload is one byte and the overhead is 32 bytes this means from 33 bytes only one byte is actual useful. 
small_memory_tree saves the payload and the childrenCount of a node in a vector. This also means that small_memory_tree can be **saved to disk relative easily**.

## How small_memory_tree saves a Tree in Memory
small_memory_tree takes the payload and the childrenCount of every node and creates a small_memory_tree::Node from it. For fast lookups small_memory_tree creates childrenOffsetEnds by running std::partial_sum over the children count of every node - NOTE: to save memory please only save the nodes.

### Example how SmallMemoryTree gets created
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
#### nodes
small_memory_tree will generate a vector called nodes. A node contains the payload and the children count of every node. In this example:  
```cpp
[{0,2},{1,1},{2,0},{3,0},{4,0}]
```
This is all small_memory_tree needs to lossless store your tree.
#### childrenOffsetEnds
small_memory_tree will run std::partial_sum over the children count of every node.
In this example:  
```cpp
[2,3,3,3,3]
```

## Memory/RAM Consumption




## Usage Example with [st_tree](https://github.com/erikerlandson/st_tree)
As always for examples look in the tests for example in test/smallMemoryTree.cxx

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
  auto treeHierarchyFromDatabase = std::vector<bool>{ true, true, true, false, false, false, false };
  auto treeDataFromDatabase = std::vector<int>{ 0, 1, 2 };
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
- Compiler with c++20 support (ranges and concepts)
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

## Use another tree library with small memory tree
small memory tree currently supports st_tree::tree and stlplus::ntree. small memory tree can be used with other tree libraries by inheriting from BaseNodeAdapter and BaseNodeAdapter from smallMemoryTreeAdapter.hxx. See stTree.hxx for an example implementation
### 

