# small_memory_tree
## Motivation and Goals
small_memory_tree saves data plus hierarchy information of a tree. It tries to **save memory** compared to other libraries who use a vector plus a pointer to the parent as a node(vecPlusPointerToParentNode). Even an empty vector needs 24 Bytes of memory (3 pointer with 8 Bytes on a 64 bit cpu) plus the pointer to the parent node which is again 8 bytes this results in 32 bytes of memory (overhead) + memory needed for the value we want to save (payload). If the payload is one byte and the overhead is 32 bytes this means from 33 bytes only one byte is actual useful. 
small_memory_tree saves the payload and the childOffset in a vector. This also means that small_memory_tree can be **saved to disk relative easily**.

## How small_memory_tree saves a Tree in Memory
### small_memory_tree class member variables
values is the result of saving the data of the tree into a vector iterating breadth-first.  
childrenOffsetEnds is the result of saving the child count of all nodes iterating breadth-first. And than running partial_sum on that result.

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
small_memory_tree will generate a vector called values by iterating breadth first over the tree and saving the node data into values. 
In this example:  
```cpp
[0,1,2,3,4]
```
small_memory_tree will generate a vector called childrenCounts by iterating breadth first over the tree and saving  the node childrenCount into childrenCounts. From childrenCount it will create childrenOffsetEnds using partial_sum on childrenCount. 
In this example:  
```cpp
[2,3,3,4,4]
```
## small_memory_tree template parameter
small_memory_tree is a class templated on ValueType and ChildrenOffsetEndType.
### ValueType
ValueType is the Type of the data of your nodes.
### ChildrenOffsetEndType
Should be an Integer which can hold the size of your tree. If your tree has 200 nodes you can use uint_8t. If your tree has 400 nodes you can use uint_16t.


## Memory/RAM Consumption
The memory consumption can be calculated with this formula:  
automatic_storage_in_byte = size_of(ValueType) + size_of(ChildrenOffsetEndType) * tree.size()
dynamic_storage_in_byte = size_of(values) + size_of(childrenOffsetEnds)

### save more memory
childrenOffsetEnds is the result of childrenCounts partial_sum. So you can reverse partial_sum to get childrenCounts from childrenOffsetEnds. childrenCounts ValueType depends on the childrenCount of your nodes. If no node of your tree has more than 255 children you can use uint_8t to store the childrenCount information.


## Usage Example with [st_tree](https://github.com/erikerlandson/st_tree)
As always for more examples look in the tests for example in test/smallMemoryTree.cxx

### create small_memory_tree and retrieve children of node using a path
```cpp
#include <iostream>
#include <small_memory_tree/stTree.hxx>
int
main ()
{
  auto stTree = st_tree::tree<int>{};
  stTree.insert (0);
  stTree.root ().insert (1);
  stTree.root ().insert (2);
  stTree.root ()[0].insert (3);
  stTree.root ()[0].insert (4);
  // create a tree using st_tree
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int>{ small_memory_tree::StTreeAdapter{ stTree } };
  // create a smallMemoryTree from a stTree using StTreeAdapter
  
  if (auto rootChildrenExpected = calcChildrenForPath (smallMemoryTree, { 0 }))
  // returns the children of the root node
    {
      auto const &rootChildren = rootChildrenExpected.value ();
      std::cout << std::format ("root has {} children\n", rootChildren.size ());
      std::ranges::for_each (rootChildren, [] (int childValue) { std::cout << std::format ("root child value {}\n", childValue); });
      if (auto child1ChildrenExpected = calcChildrenForPath (smallMemoryTree, { 0, 1 }))
        {
          std::cout << std::format ("child1 has {} children\n", rootChildren.size ());
          auto const &child1Children = child1ChildrenExpected.value ();
          std::ranges::for_each (child1Children, [] (int childValue) { std::cout << std::format ("child1 child value {}\n", childValue); });
        }
      else
        {
          std::cout << child1ChildrenExpected.error () + '\n';
        }
    }
  else
    {
      std::cout << rootChildrenExpected.error () + '\n';
    }
  return 0;
}
```

### save small_memory_tree to database and restore small_memory_tree
```cpp
#include <small_memory_tree/stTree.hxx>
int
main ()
{
  // create a tree using st_tree
  auto stTree = st_tree::tree<uint8_t>{};
  stTree.insert (0);
  stTree.root ().insert (1);
  stTree.root ().insert (2);
  stTree.root ()[0].insert (3);
  stTree.root ()[0].insert (4);
  // create a smallMemoryTree from a stTree using StTreeAdapter
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<uint8_t>{ small_memory_tree::StTreeAdapter{ stTree } };
  // store values and childrenOffsetEnds in your database
  auto const &values = smallMemoryTree.getValues ();
  auto const &childrenOffsetEnds = smallMemoryTree.getChildrenOffsetEnds ();
  //  later in your code retrieve values and childrenOffsetEnds from your database
  auto const &valuesFromDatabase = std::vector<uint8_t>{ 0, 1, 2, 3, 4 };
  auto const &childrenOffsetEndsFromDatabase = std::vector<uint8_t>{ 2, 4, 4, 4, 4 };
  //  create smallMemoryTree with data from database
  auto smallMemoryTreeFromDatabase = small_memory_tree::SmallMemoryTree<uint8_t, uint8_t>{ valuesFromDatabase, childrenOffsetEndsFromDatabase };
  return 0;
}
```

## Build
### Requirements
- Compiler with c++23 support (std::expected)
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

