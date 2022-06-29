#ifndef CD77E88E_7A82_4A96_B983_3A9338969CDC
#define CD77E88E_7A82_4A96_B983_3A9338969CDC

#include "src/smallMemoryTree.hxx"
#include "src/util.hxx"
#include "src/vectorToTree.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <cstddef>
#include <exception>
#include <functional>
#include <optional>
#include <range/v3/algorithm/find_if.hpp>
#include <st_tree.h>
#include <stdexcept>
#include <tuple>
#include <vector>
template <typename T>
std::vector<T>
treeToVector (auto const &tree, T const &markerForEmpty, T const &markerForChild, std::function<typename std::decay<decltype (markerForEmpty)>::type (typename std::decay<decltype (*tree.begin ())>::type const &node)> nodeToData = {})
{
  auto const maxChildrenInTree = maxChildren (tree);
  auto result = std::vector<T>{};
  for (auto &node : tree)
    {
      if (nodeToData)
        {
          result.push_back (nodeToData (node));
        }
      else
        {
          if constexpr (std::is_same<typename std::decay<decltype (node.data ())>::type, T>::value)
            {
              result.push_back (node.data ());
            }
        }
      auto currentChildren = size_t{};
      while (currentChildren < node.size ())
        {
          result.push_back (markerForChild);
          currentChildren++;
        }
      while (currentChildren < maxChildrenInTree)
        {
          result.push_back (markerForEmpty);
          currentChildren++;
        }
    }
  result.shrink_to_fit ();
  fillChildren (result, maxChildrenInTree, markerForChild);
  return result;
}

#endif /* CD77E88E_7A82_4A96_B983_3A9338969CDC */
