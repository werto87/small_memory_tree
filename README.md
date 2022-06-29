# small_memory_tree  
Compress st_tree. Decompress compressed st_tree.  
Use the function XYZ to create a small_memory_tree from a st_tree.  
Use the function ZYX to create a st_tree from a small_memory_tree.  

## Difference between st_tree and small_memory_tree
st_tree goals from [st_tree git readme](https://github.com/erikerlandson/st_tree)
>The st_tree library allows the programmer to easily declare and manipulate data in a tree  

The goal of small_memory_tree is lossless compression of a st_tree.

## Use cases
- You want to save memory
- You want to save a st_tree to disk

## Do not use small_memory_tree if
- Your data on the nodes does not allow to set two marks. You should have 2 free values which small_memory_tree can use as marks
- You have a st_tree with a lot of nodes with a low amount of children and a few nodes with a lot of children. In this case small_memory_tree could get bigger than st_tree
- Your data type is small for example uint8_t and you have a lot of nodes.

## How does small_memory_tree stores a tree
small_memory_tree stores the st_tree into a vector. It preserves the hierarchic information in a tree by using indexes.
Example:
Tree:
          0  
         / \  
        1   2  
       /  
      3  
     /  
   4  
   
Vector:  
00-0-----------root       
01-3-----------index child 1  
02-6-----------index child 2  
03-1-----------child value 1  
04-9-----------index child 3  
05-255--------marker for empty value  
06-2-----------child value 2  
07-255--------marker for empty value  
08-255--------marker for empty value  
09-3-----------child value 3  
10-12----------index child 4  
11-255--------marker for empty value  
12-4-----------child value 4  
13-255--------marker for empty value  
14-255--------marker for empty value  
