# small_memory_tree  
Compress sl_tree. Decompress compressed sl_tree.  
Use the function XYZ to create a small_memory_tree from a sl_tree.  
Use the function ZYX to create a sl_tree from a small_memory_tree.  

## Difference between sl_tree and small_memory_tree
sl_tree goals from [sl_tree git readme](https://github.com/erikerlandson/st_tree)
>The st_tree library allows the programmer to easily declare and manipulate data in a tree  

The goal of small_memory_tree is lossless compression of a sl_tree.

## Use cases
- You want to save memory
- You want to save a sl_tree to disk

## Do not use small_memory_tree if
- Your data on the nodes does not allow to set two marks. You should have 2 free values which small_memory_tree can use as marks.
- You have a sl_tree with a lot of nodes with a low amount of children and a few nodes with a lot of children. In this case small_memory_tree could get bigger than sl_tree


## How does small_memory_tree stores a tree
[write this after implementation]

