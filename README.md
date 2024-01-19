# small_memory_tree

small_memory_tree is a library for compressing a st_tree to save memory (depending on the structure of your tree. Read "
Do not use small_memory_tree if" section").
small_memory_tree saves the data plus hierarchy information to a std::vector for more details read "How does
small_memory_tree store a tree"
section.

## Requirements

The st_tree should have a numeric type (for example st_tree<uint8_t>) or a pair where first is a numeric type and second
can be any user defined type (for example st_tree<uint8_t,MyUserDefinedType>).

## How to use

```cpp
---------0
-------/---\
------1-----2
-----/
---3
--/
-4
*/
auto tree = st_tree::tree<uint8_t>{}; // create a tree
tree.insert (0); // fill it
tree.root ().insert (1);
tree.root ().insert (2);
tree.root ()[0].insert (3);
tree.root ()[0][0].insert (4);
auto compressedTree = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 }); // creates a vector with the data from tree using 255 as marker for empty and 254 for child
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

## How does small_memory_tree store a tree

small_memory_tree stores the st_tree into a vector. It preserves the hierarchic information by creating additional
elements.

Example:
Tree:  
-----------0  
---------/---\  
--------1---2  
-------/  
-----3  
----/  
---4

Vector:  
00-0-----------root       
01-2-----------distance to child 1  
02-4-----------distance to child 2  
03-1-----------child value 1  
04-5-----------distance to child 3  
05-255--------marker for empty value  
06-2-----------child value 2  
07-255--------marker for empty value  
08-255--------marker for empty value  
09-3-----------child value 3  
10-2-----------distance to child 4  
11-255--------marker for empty value  
12-4-----------child value 4  
13-255--------marker for empty value  
14-255--------marker for empty value  
