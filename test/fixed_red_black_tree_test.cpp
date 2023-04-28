#include "fixed_containers/fixed_red_black_tree.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <queue>
#include <random>

namespace fixed_containers::fixed_red_black_tree_detail
{
namespace
{
static_assert(IsStructuralType<FixedIndexBasedPoolStorage<int, 5>>);
static_assert(IsStructuralType<FixedIndexBasedContiguousStorage<int, 5>>);

static_assert(IsStructuralType<NodeIndexWithColorEmbeddedInTheMostSignificantBit>);

static_assert(IsRedBlackTreeNode<DefaultRedBlackTreeNode<int, EmptyValue>>);
static_assert(IsRedBlackTreeNodeWithValue<DefaultRedBlackTreeNode<int, double>>);
static_assert(IsRedBlackTreeNode<CompactRedBlackTreeNode<int, EmptyValue>>);
static_assert(IsRedBlackTreeNodeWithValue<CompactRedBlackTreeNode<int, double>>);

static_assert(IsStructuralType<DefaultRedBlackTreeNode<int, EmptyValue>, 5>);
static_assert(IsStructuralType<DefaultRedBlackTreeNode<int, int>, 5, 99>);
static_assert(IsStructuralType<CompactRedBlackTreeNode<int, EmptyValue>, 5>);
static_assert(IsStructuralType<CompactRedBlackTreeNode<int, int>, 5, 99>);

static_assert(
    IsRedBlackTreeNodeWithValue<RedBlackTreeNodeView<CompactRedBlackTreeNode<int, EmptyValue>>>);

using Storage_1 = FixedRedBlackTreeStorage<int,
                                           double,
                                           10,
                                           RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
                                           FixedIndexBasedPoolStorage>;
static_assert(IsFixedRedBlackTreeStorage<Storage_1>);
static_assert(IsStructuralType<Storage_1>);

using ES_1 = FixedRedBlackTree<int, int, 10>;
static_assert(TriviallyCopyable<ES_1>);
static_assert(NotTrivial<ES_1>);
static_assert(StandardLayout<ES_1>);
static_assert(TriviallyCopyAssignable<ES_1>);
static_assert(TriviallyMoveAssignable<ES_1>);
static_assert(IsStructuralType<ES_1>);

template <typename K>
constexpr CompactRedBlackTreeNode<K> make_node(
    const K& key, NodeIndex parent_index, NodeIndex left_index, NodeIndex right_index, Color color)
{
    CompactRedBlackTreeNode<K> out{key};
    out.set_parent_index(parent_index);
    out.set_left_index(left_index);
    out.set_right_index(right_index);
    out.set_color(color);
    return out;
}

template <typename K, typename V>
constexpr CompactRedBlackTreeNode<K, V> make_node(const K& key,
                                                  const V& value,
                                                  NodeIndex parent_index,
                                                  NodeIndex left_index,
                                                  NodeIndex right_index,
                                                  Color color)
{
    CompactRedBlackTreeNode<K, V> out{key, value};
    out.set_parent_index(parent_index);
    out.set_left_index(left_index);
    out.set_right_index(right_index);
    out.set_color(color);
    return out;
}

template <IsRedBlackTreeNode NodeTypeA, IsRedBlackTreeNode NodeTypeB>
constexpr bool are_equal_impl(const NodeTypeA& a, const NodeTypeB& b)
{
    auto left = std::tuple(a.key(), a.color(), a.parent_index(), a.left_index(), a.right_index());
    auto right = std::tuple(b.key(), b.color(), b.parent_index(), b.left_index(), b.right_index());

    if (left != right)
    {
        std::cout << a.key() << ", " << a.color() << ", " << a.parent_index() << ", "
                  << a.left_index() << ", " << a.right_index() << std::endl;
        std::cout << b.key() << ", " << b.color() << ", " << b.parent_index() << ", "
                  << b.left_index() << ", " << b.right_index() << std::endl;
        return false;
    }

    return true;
}

template <IsRedBlackTreeNodeWithValue NodeTypeA, IsRedBlackTreeNodeWithValue NodeTypeB>
constexpr bool are_equal_impl(const NodeTypeA& a, const NodeTypeB& b)
{
    auto left = std::tuple(
        a.key(), a.value(), a.color(), a.parent_index(), a.left_index(), a.right_index());
    auto right = std::tuple(
        b.key(), b.value(), b.color(), b.parent_index(), b.left_index(), b.right_index());

    if (left != right)
    {
        std::cout << a.key() << ", " << a.value() << ", " << a.color() << ", " << a.parent_index()
                  << ", " << a.left_index() << ", " << a.right_index() << std::endl;
        std::cout << b.key() << ", " << b.value() << ", " << b.color() << ", " << b.parent_index()
                  << ", " << b.left_index() << ", " << b.right_index() << std::endl;
        return false;
    }

    return true;
}

template <IsRedBlackTreeNode NodeTypeA, IsRedBlackTreeNode NodeTypeB>
constexpr bool are_equal(const NodeTypeA& a, const NodeTypeB& b)
{
    return are_equal_impl(a, b);
}

template <class TreeType, class ArrayType>
constexpr bool contains_all_from_to(const TreeType& tree,
                                    const ArrayType& arr,
                                    const std::size_t from,
                                    const std::size_t to)
{
    for (std::size_t i = from; i < to; i++)
    {
        if (!tree.contains_node(arr[i]))
        {
            return false;
        }
    }
    return true;
}

template <class TreeStorageType>
std::size_t find_height(const TreeStorageType& tree_storage, const NodeIndex& root_index)
{
    static constexpr NodeIndex HEIGHT_MARKER = NULL_INDEX - 1;
    if (root_index == NULL_INDEX)
    {
        return 0;
    }

    std::queue<NodeIndex> q{};
    std::size_t height = 0;
    q.push(root_index);
    q.push(HEIGHT_MARKER);
    while (!q.empty())
    {
        const NodeIndex i = q.front();
        q.pop();
        if (i == HEIGHT_MARKER)
        {
            height++;
            // Unless we are done, add another marker
            if (!q.empty())
            {
                q.push(HEIGHT_MARKER);
            }
            continue;
        }

        const auto& node = tree_storage.node_at(i);
        if (node.left_index() != NULL_INDEX)
        {
            q.push(node.left_index());
        }
        if (node.right_index() != NULL_INDEX)
        {
            q.push(node.right_index());
        }
    }
    return height - 1;
}

template <class TreeStorageType>
std::size_t find_height(const TreeStorageType& tree_storage)
{
    return find_height(tree_storage, tree_storage.root_index());
}

std::size_t max_height_of_red_black_tree(const std::size_t size)
{
    // https://stackoverflow.com/questions/43529279/how-to-create-red-black-tree-with-max-height
    return 2 * static_cast<std::size_t>(std::log2(size + 1));
}

}  // namespace

TEST(NodeIndexWithColorEmbeddedInTheMostSignificantBit, Basic)
{
    {
        constexpr auto default_value = []()
        { return NodeIndexWithColorEmbeddedInTheMostSignificantBit{}; }();
        static_assert(consteval_compare::equal<NULL_INDEX, default_value.get_index()>);
        static_assert(consteval_compare::equal<BLACK, default_value.get_color()>);
    }

    {
        constexpr auto set_value_with_black = []()
        {
            NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
            ret.set_index(365);
            ret.set_color(BLACK);
            return ret;
        }();
        constexpr auto set_value_with_red = []()
        {
            NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
            ret.set_index(365);
            ret.set_color(RED);
            return ret;
        }();

        static_assert(consteval_compare::equal<365, set_value_with_black.get_index()>);
        static_assert(consteval_compare::equal<BLACK, set_value_with_black.get_color()>);

        static_assert(consteval_compare::equal<365, set_value_with_red.get_index()>);
        static_assert(consteval_compare::equal<RED, set_value_with_red.get_color()>);
    }

    {
        constexpr auto set_min_value_with_black = []()
        {
            NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
            ret.set_index(0);
            ret.set_color(BLACK);
            return ret;
        }();
        constexpr auto set_min_value_with_red = []()
        {
            NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
            ret.set_index(0);
            ret.set_color(RED);
            return ret;
        }();

        static_assert(consteval_compare::equal<0, set_min_value_with_black.get_index()>);
        static_assert(consteval_compare::equal<BLACK, set_min_value_with_black.get_color()>);

        static_assert(consteval_compare::equal<0, set_min_value_with_red.get_index()>);
        static_assert(consteval_compare::equal<RED, set_min_value_with_red.get_color()>);
    }

    {
        static constexpr NodeIndex MAX_INDEX = NULL_INDEX / 2;
        constexpr auto set_max_value_with_black = []()
        {
            NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
            ret.set_index(MAX_INDEX);
            ret.set_color(BLACK);
            return ret;
        }();
        constexpr auto set_max_value_with_red = []()
        {
            NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
            ret.set_index(MAX_INDEX);
            ret.set_color(RED);
            return ret;
        }();

        static_assert(consteval_compare::equal<NULL_INDEX, set_max_value_with_black.get_index()>);
        static_assert(consteval_compare::equal<BLACK, set_max_value_with_black.get_color()>);

        static_assert(consteval_compare::equal<NULL_INDEX, set_max_value_with_red.get_index()>);
        static_assert(consteval_compare::equal<RED, set_max_value_with_red.get_color()>);

        NodeIndexWithColorEmbeddedInTheMostSignificantBit ret{};
        EXPECT_DEATH(ret.set_index(MAX_INDEX + 1), "");
    }
}

TEST(DefaultRedBlackTreeNode, Construction)
{
    // Without Value
    {
        constexpr DefaultRedBlackTreeNode<int> a1{5};
        static_assert(consteval_compare::equal<5, a1.key()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
    {
        constexpr int key = 5;
        constexpr DefaultRedBlackTreeNode<int> a1{key};
        static_assert(consteval_compare::equal<key, a1.key()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }

    // With Value
    {
        constexpr DefaultRedBlackTreeNode<int, int> a1{5, 15};
        static_assert(consteval_compare::equal<5, a1.key()>);
        static_assert(consteval_compare::equal<15, a1.value()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
    {
        constexpr int key = 5;
        constexpr DefaultRedBlackTreeNode<int, int> a1{key, 15};
        static_assert(consteval_compare::equal<key, a1.key()>);
        static_assert(consteval_compare::equal<15, a1.value()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
    {
        constexpr DefaultRedBlackTreeNode<int, TypeWithMultipleConstructorParameters> a1{
            5,
            /*ImplicitlyConvertibleFromInt*/ 100,
            ExplicitlyConvertibleFromInt{200}};
        static_assert(consteval_compare::equal<5, a1.key()>);
        static_assert(consteval_compare::equal<100, a1.value().implicit_int.value>);
        static_assert(consteval_compare::equal<200, a1.value().explicit_int.value>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
}

TEST(CompactRedBlackTreeNode, Construction)
{
    // Without Value
    {
        constexpr CompactRedBlackTreeNode<int> a1{5};
        static_assert(consteval_compare::equal<5, a1.key()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
    {
        constexpr int key = 5;
        constexpr CompactRedBlackTreeNode<int> a1{key};
        static_assert(consteval_compare::equal<key, a1.key()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }

    // With Value
    {
        constexpr CompactRedBlackTreeNode<int, int> a1{5, 15};
        static_assert(consteval_compare::equal<5, a1.key()>);
        static_assert(consteval_compare::equal<15, a1.value()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
    {
        constexpr int key = 5;
        constexpr CompactRedBlackTreeNode<int, int> a1{key, 15};
        static_assert(consteval_compare::equal<key, a1.key()>);
        static_assert(consteval_compare::equal<15, a1.value()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
    {
        constexpr CompactRedBlackTreeNode<int, TypeWithMultipleConstructorParameters> a1{
            5,
            /*ImplicitlyConvertibleFromInt*/ 100,
            ExplicitlyConvertibleFromInt{200}};
        static_assert(consteval_compare::equal<5, a1.key()>);
        static_assert(consteval_compare::equal<100, a1.value().implicit_int.value>);
        static_assert(consteval_compare::equal<200, a1.value().explicit_int.value>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.parent_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.left_index()>);
        static_assert(consteval_compare::equal<NULL_INDEX, a1.right_index()>);
    }
}

TEST(FixedRedBlackTreeSet, NoValue)
{
    FixedRedBlackTreeSet<int, 10> bst;

    {
        bst.insert_node(15);  // Position 0
        ASSERT_EQ(1, bst.size());
        ASSERT_EQ(0, find_height(bst));
        // Position 0 associated with (15, 15)

        ASSERT_TRUE(
            are_equal(make_node(15, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(0)));
    }

    {
        // bst.insert_node(15);  // Position 0
        bst.insert_node(5);  // Position 1
        ASSERT_EQ(2, bst.size());
        ASSERT_EQ(1, find_height(bst));

        /*
         *               18B
         *             /
         *           5R
         */

        ASSERT_TRUE(are_equal(make_node(15, NULL_INDEX, 1, NULL_INDEX, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
    }

    {
        // bst.insert_node(15);  // Position 0
        // bst.insert_node(5);  // Position 1
        bst.insert_node(1);  // Position 3
        ASSERT_EQ(3, bst.size());
        ASSERT_EQ(1, find_height(bst));

        /*
         *               5B
         *             /   \
         *           1R     15R
         */
        ASSERT_TRUE(are_equal(make_node(15, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, NULL_INDEX, 2, 0, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(1, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
    }
}

TEST(FixedRedBlackTree, InsertionExample1)
{
    FixedRedBlackTree<int, int, 10> bst;

    {
        bst[15] = 150;  // Position 0
        ASSERT_EQ(1, bst.size());
        ASSERT_EQ(0, find_height(bst));
        // Position 0 associated with (15, 15)

        ASSERT_TRUE(are_equal(make_node(15, 150, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK),
                              bst.node_at(0)));
    }

    {
        // bst[15] = 150;  // Position 0
        bst[5] = 50;  // Position 1
        ASSERT_EQ(2, bst.size());
        ASSERT_EQ(1, find_height(bst));

        /*
         *               18B
         *             /
         *           5R
         */

        ASSERT_TRUE(
            are_equal(make_node(15, 150, NULL_INDEX, 1, NULL_INDEX, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
    }

    {
        // bst[15] = 150;  // Position 0
        // bst[5] = 50;  // Position 1
        bst[1] = 10;  // Position 3
        ASSERT_EQ(3, bst.size());
        ASSERT_EQ(1, find_height(bst));

        /*
         *               5B
         *             /   \
         *           1R     15R
         */
        ASSERT_TRUE(are_equal(make_node(15, 150, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, 50, NULL_INDEX, 2, 0, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
    }
}

TEST(FixedRedBlackTree, InsertionExample2)
{
    FixedRedBlackTree<int, int, 20> bst;

    {
        bst[8] = 80;    // Position 0
        bst[5] = 50;    // Position 1
        bst[15] = 150;  // Position 2
        bst[12] = 120;  // Position 3
        bst[19] = 190;  // Position 4
        bst[9] = 90;    // Position 5
        bst[13] = 130;  // Position 6
        bst[23] = 230;  // Position 7
        ASSERT_EQ(8, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               8B
         *             /    \
         *           5B      15R
         *                 /     \
         *               12B     19B
         *             /    \       \
         *           9R     13R      23R
         */

        ASSERT_TRUE(are_equal(make_node(8, 80, NULL_INDEX, 1, 2, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, 3, 4, RED), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(12, 120, 2, 5, 6, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 2, NULL_INDEX, 7, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 3, NULL_INDEX, NULL_INDEX, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 3, NULL_INDEX, NULL_INDEX, RED), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(23, 230, 4, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
    }
}

TEST(FixedRedBlackTree, Insertion_FocusOnTheRight)
{
    FixedRedBlackTree<int, int, 20> bst;

    // Starting State
    {
        bst[3] = 30;  // Position 0
        bst[1] = 10;  // Position 1
        bst[5] = 50;  // Position 2
        ASSERT_EQ(3, bst.size());
        ASSERT_EQ(1, find_height(bst));

        /*
         *               3B
         *             /    \
         *           1R      5R
         */

        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 1, 2, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
    }

    // color-flip
    {
        // bst[3] = 30;  // Position 0
        // bst[1] = 10;  // Position 1
        // bst[5] = 50;  // Position 2
        bst[7] = 70;  // Position 3
        ASSERT_EQ(4, bst.size());
        ASSERT_EQ(2, find_height(bst));

        /*
         *               3B
         *             /    \
         *           1B      5B
         *                     \
         *                       7R
         */

        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 1, 2, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, 3, BLACK), bst.node_at(2)));
    }
    // right-left rotation
    {
        // bst[3] = 30;  // Position 0
        // bst[1] = 10;  // Position 1
        // bst[5] = 50;  // Position 2
        // bst[7] = 70;  // Position 3
        bst[6] = 60;  // Position 4
        ASSERT_EQ(5, bst.size());
        ASSERT_EQ(2, find_height(bst));

        /*
         *               3B
         *             /    \
         *           1B      6B
         *                  /  \
         *                5R    7R
         */

        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 1, 4, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 4, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 4, NULL_INDEX, NULL_INDEX, RED), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, 0, 2, 3, BLACK), bst.node_at(4)));
    }

    // color-flip
    {
        // bst[3] = 30;  // Position 0
        // bst[1] = 10;  // Position 1
        // bst[5] = 50;  // Position 2
        // bst[7] = 70;  // Position 3
        // bst[6] = 60;  // Position 4
        bst[8] = 80;  // Position 5
        ASSERT_EQ(6, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               3B
         *             /    \
         *           1B      6R
         *                  /  \
         *                5B    7B
         *                        \
         *                         8R
         */

        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 1, 4, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 4, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 4, NULL_INDEX, 5, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, 0, 2, 3, RED), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 3, NULL_INDEX, NULL_INDEX, RED), bst.node_at(5)));
    }

    // left rotation
    {
        // bst[3] = 30;  // Position 0
        // bst[1] = 10;  // Position 1
        // bst[5] = 50;  // Position 2
        // bst[7] = 70;  // Position 3
        // bst[6] = 60;  // Position 4
        // bst[8] = 80;  // Position 5
        bst[9] = 90;  // Position 6
        ASSERT_EQ(7, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               3B
         *             /    \
         *           1B      6R
         *                  /  \
         *                5B    8B
         *                     /  \
         *                    7R   9R
         */

        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 1, 4, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 4, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, RED), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, 0, 2, 5, RED), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, BLACK), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, NULL_INDEX, RED), bst.node_at(6)));
    }

    // color flip + left rotation
    {
        // bst[3] = 30;  // Position 0
        // bst[1] = 10;  // Position 1
        // bst[5] = 50;  // Position 2
        // bst[7] = 70;  // Position 3
        // bst[6] = 60;  // Position 4
        // bst[8] = 80;  // Position 5
        // bst[9] = 90;  // Position 6
        bst[10] = 100;  // Position 7
        ASSERT_EQ(8, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */

        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 1, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, 7, BLACK), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
    }
}

// This is symmetric to Example3: for every key x do (20 - x) instead
TEST(FixedRedBlackTree, Insertion_FocusOnTheLeft)
{
    FixedRedBlackTree<int, int, 20> bst;

    // Starting State
    {
        bst[17] = 170;  // Position 0
        bst[19] = 190;  // Position 1
        bst[15] = 150;  // Position 2
        ASSERT_EQ(3, bst.size());
        ASSERT_EQ(1, find_height(bst));

        /*
         *               17B
         *             /    \
         *           15R      19R
         */

        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 2, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
    }

    // color-flip
    {
        //        bst[17] = 170;  // Position 0
        //        bst[19] = 190;  // Position 1
        //        bst[15] = 150;  // Position 2
        bst[13] = 130;  // Position 3
        ASSERT_EQ(4, bst.size());
        ASSERT_EQ(2, find_height(bst));

        /*
         *               17B
         *             /    \
         *           15B      19B
         *          /
         *        13R
         */

        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 2, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(
            are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, 3, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 2, NULL_INDEX, NULL_INDEX, RED), bst.node_at(3)));
    }
    // left-right rotation
    {
        // bst[17] = 170;  // Position 0
        // bst[19] = 190;  // Position 1
        // bst[15] = 150;  // Position 2
        // bst[13] = 130;  // Position 3
        bst[14] = 140;  // Position 4
        ASSERT_EQ(5, bst.size());
        ASSERT_EQ(2, find_height(bst));

        /*
         *               17B
         *             /    \
         *           14B      19B
         *          /   \
         *        13R   15B
         */

        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 4, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(
            are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 4, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 4, NULL_INDEX, NULL_INDEX, RED), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(14, 140, 0, 3, 2, BLACK), bst.node_at(4)));
    }

    // color-flip
    {
        // bst[17] = 170;  // Position 0
        // bst[19] = 190;  // Position 1
        // bst[15] = 150;  // Position 2
        // bst[13] = 130;  // Position 3
        // bst[14] = 140;  // Position 4
        bst[12] = 120;  // Position 5
        ASSERT_EQ(6, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               17B
         *             /    \
         *           14R      19B
         *          /   \
         *        13B   15B
         *       /
         *     12R
         */

        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 4, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(
            are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(
            are_equal(make_node(15, 150, 4, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 4, 5, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(14, 140, 0, 3, 2, RED), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(12, 120, 3, NULL_INDEX, NULL_INDEX, RED), bst.node_at(5)));
    }

    // right rotation
    {
        // bst[17] = 170;  // Position 0
        // bst[19] = 190;  // Position 1
        // bst[15] = 150;  // Position 2
        // bst[13] = 130;  // Position 3
        // bst[14] = 140;  // Position 4
        // bst[12] = 120;  // Position 5
        bst[11] = 110;  // Position 6
        ASSERT_EQ(7, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               17B
         *             /    \
         *           14R      19B
         *          /   \
         *        12B   15B
         *       /   \
         *     11R   13R
         */

        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 4, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(
            are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(
            are_equal(make_node(15, 150, 4, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 5, NULL_INDEX, NULL_INDEX, RED), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(14, 140, 0, 5, 2, RED), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(12, 120, 4, 6, 3, BLACK), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(11, 110, 5, NULL_INDEX, NULL_INDEX, RED), bst.node_at(6)));
    }

    // color flip + right rotation
    {
        // bst[17] = 170;  // Position 0
        // bst[19] = 190;  // Position 1
        // bst[15] = 150;  // Position 2
        // bst[13] = 130;  // Position 3
        // bst[14] = 140;  // Position 4
        // bst[12] = 120;  // Position 5
        // bst[11] = 110;  // Position 6
        bst[10] = 100;  // Position 7
        ASSERT_EQ(8, bst.size());
        ASSERT_EQ(3, find_height(bst));

        /*
         *               14B
         *             /    \
         *           12R      17B
         *          /   \    /    \
         *        11B   13B 15B   19B
         *       /
         *     10R
         */

        ASSERT_TRUE(are_equal(make_node(17, 170, 4, 2, 1, RED), bst.node_at(0)));
        ASSERT_TRUE(
            are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(
            are_equal(make_node(15, 150, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(
            are_equal(make_node(13, 130, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(14, 140, NULL_INDEX, 5, 0, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(12, 120, 4, 6, 3, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(11, 110, 5, 7, NULL_INDEX, BLACK), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
    }
}

static FixedRedBlackTree<int, int, 7> get_new_swap_test_base_tree()
{
    FixedRedBlackTree<int, int, 7> bst{};
    bst[17] = 170;  // Position 0
    bst[19] = 190;  // Position 1
    bst[15] = 150;  // Position 2
    return bst;
}

TEST(FixedRedBlackTree, SwapNodes)
{
    using Ops = FixedRedBlackTreeOps<FixedRedBlackTree<int, int, 7>>;
    // Swap non-neighbors #1
    {
        /*
         *               17B
         *             /    \
         *           15R      19R
         */
        auto bst = get_new_swap_test_base_tree();

        //        bst[17] = 170;  // Position 0
        //        bst[19] = 190;  // Position 1
        //        bst[15] = 150;  // Position 2
        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 2, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
        Ops::swap_nodes_including_key_and_value(bst, 1, 2);
        //        bst[17] = 170;  // Position 0
        //        bst[15] = 150;  // Position 1
        //        bst[19] = 190;  // Position 2
        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 1, 2, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));

        Ops::swap_nodes_including_key_and_value(bst, 2, 1);
        auto original_bst = get_new_swap_test_base_tree();
        ASSERT_TRUE(are_equal(original_bst.node_at(0), bst.node_at(0)));
        ASSERT_TRUE(are_equal(original_bst.node_at(1), bst.node_at(1)));
        ASSERT_TRUE(are_equal(original_bst.node_at(2), bst.node_at(2)));
    }

    // Swap left-child/parent
    {
        /*
         *               17B
         *             /    \
         *           15R      19R
         */
        auto bst = get_new_swap_test_base_tree();
        //        bst[17] = 170;  // Position 0
        //        bst[19] = 190;  // Position 1
        //        bst[15] = 150;  // Position 2
        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 2, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
        Ops::swap_nodes_including_key_and_value(bst, 2, 0);
        //        bst[15] = 150;  // Position 0
        //        bst[19] = 190;  // Position 1
        //        bst[17] = 170;  // Position 2
        ASSERT_TRUE(are_equal(make_node(15, 150, 2, NULL_INDEX, NULL_INDEX, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 2, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 0, 1, BLACK), bst.node_at(2)));

        Ops::swap_nodes_including_key_and_value(bst, 0, 2);
        auto original_bst = get_new_swap_test_base_tree();
        ASSERT_TRUE(are_equal(original_bst.node_at(0), bst.node_at(0)));
        ASSERT_TRUE(are_equal(original_bst.node_at(1), bst.node_at(1)));
        ASSERT_TRUE(are_equal(original_bst.node_at(2), bst.node_at(2)));
    }

    // Swap right-child/parent
    {
        /*
         *               17B
         *             /    \
         *           15R      19R
         */
        auto bst = get_new_swap_test_base_tree();
        //        bst[17] = 170;  // Position 0
        //        bst[19] = 190;  // Position 1
        //        bst[15] = 150;  // Position 2
        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 2, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(19, 190, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
        Ops::swap_nodes_including_key_and_value(bst, 1, 0);
        //        bst[19] = 190;  // Position 0
        //        bst[17] = 170;  // Position 1
        //        bst[15] = 150;  // Position 2
        ASSERT_TRUE(are_equal(make_node(19, 190, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(17, 170, NULL_INDEX, 2, 0, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(15, 150, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));

        Ops::swap_nodes_including_key_and_value(bst, 0, 1);
        auto original_bst = get_new_swap_test_base_tree();
        ASSERT_TRUE(are_equal(original_bst.node_at(0), bst.node_at(0)));
        ASSERT_TRUE(are_equal(original_bst.node_at(1), bst.node_at(1)));
        ASSERT_TRUE(are_equal(original_bst.node_at(2), bst.node_at(2)));
    }
}

static FixedRedBlackTree<int, int, 20> get_new_deletion_test_base_tree()
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[3] = 30;    // Position 0
    bst[1] = 10;    // Position 1
    bst[5] = 50;    // Position 2
    bst[7] = 70;    // Position 3
    bst[6] = 60;    // Position 4
    bst[8] = 80;    // Position 5
    bst[9] = 90;    // Position 6
    bst[10] = 100;  // Position 7
    return bst;
}

TEST(FixedRedBlackTree, Deletion)
{
    // Base verification
    {
        auto bst = get_new_deletion_test_base_tree();
        ASSERT_EQ(8, bst.size());

        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */

        ASSERT_EQ(3, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 1, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, 7, BLACK), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
    }

    // Last entry + no children
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1
        //        bst[5] = 50;    // Position 2
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4
        //        bst[8] = 80;    // Position 5
        //        bst[9] = 90;    // Position 6
        //        bst[10] = 100;  // Position 7 - Delete
        bst.delete_node(10);
        ASSERT_EQ(7, bst.size());

        /*
         *               5B
         *             /    \
         *           3B      8R
         *          /       /   \
         *         1R      7B    9B
         */

        ASSERT_EQ(2, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 1, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(6)));
    }

    // non-last entry, no children, is a left child
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1 - Deleted
        //        bst[5] = 50;    // Position 2
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4
        //        bst[8] = 80;    // Position 5
        //        bst[9] = 90;    // Position 6
        //        bst[10] = 100;  // Position 7
        bst.delete_node(1);
        ASSERT_EQ(7, bst.size());

        /*
         *               6B
         *             /    \
         *           3B      8R
         *             \    /   \
         *             5R  7B    9B
         *                         \
         *                          10R
         */

        ASSERT_EQ(3, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, NULL_INDEX, 2, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, 7, BLACK), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
    }

    // non-last entry, no children, is a right child
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1
        //        bst[5] = 50;    // Position 2 - Deleted
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4
        //        bst[8] = 80;    // Position 5
        //        bst[9] = 90;    // Position 6
        //        bst[10] = 100;  // Position 7
        bst.delete_node(5);
        ASSERT_EQ(7, bst.size());

        /*
         *               6B
         *             /    \
         *           3B      8R
         *          /       /   \
         *         1R      7B    9B
         *                         \
         *                          10R
         */

        ASSERT_EQ(3, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 1, NULL_INDEX, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, 7, BLACK), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
    }

    // only has right child
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1
        //        bst[5] = 50;    // Position 2
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4
        //        bst[8] = 80;    // Position 5
        //        bst[9] = 90;    // Position 6 - Deleted
        //        bst[10] = 100;  // Position 7
        bst.delete_node(9);
        ASSERT_EQ(7, bst.size());

        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    10B
         */

        ASSERT_EQ(2, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 1, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 7, RED), bst.node_at(5)));
        ASSERT_TRUE(
            are_equal(make_node(10, 100, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(7)));
    }

    // Only has left child
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *        /                \
         *       0B                 10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1 - Deleted
        //        bst[5] = 50;    // Position 2
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4
        //        bst[8] = 80;    // Position 5
        //        bst[9] = 90;    // Position 6
        //        bst[10] = 100;  // Position 7
        bst[0] = 42;  // Position 8
        bst.delete_node(1);
        ASSERT_EQ(8, bst.size());

        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         0B  5B  7B    9B
         *                         \
         *                          10R
         */

        ASSERT_EQ(3, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 8, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 5, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 5, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 4, 3, 6, RED), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 5, NULL_INDEX, 7, BLACK), bst.node_at(6)));
        ASSERT_TRUE(are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, RED), bst.node_at(7)));
        ASSERT_TRUE(are_equal(make_node(0, 42, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(8)));
    }

    // Two children and is not the root
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1
        //        bst[5] = 50;    // Position 2
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4
        //        bst[8] = 80;    // Position 5 - Deleted
        //        bst[9] = 90;    // Position 6
        //        bst[10] = 100;  // Position 7
        bst.delete_node(8);
        ASSERT_EQ(7, bst.size());

        /*
         *               6B
         *             /    \
         *           3R      9R
         *          /  \    /   \
         *         1B  5B 7B    10B
         */

        ASSERT_EQ(2, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 4, 1, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, 6, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(6, 60, NULL_INDEX, 0, 6, BLACK), bst.node_at(4)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 4, 3, 7, RED), bst.node_at(6)));
        ASSERT_TRUE(
            are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(7)));
    }

    // Two children and is the root
    {
        /*
         *               6B
         *             /    \
         *           3R      8R
         *          /  \    /   \
         *         1B  5B  7B    9B
         *                         \
         *                          10R
         */
        auto bst = get_new_deletion_test_base_tree();
        //        bst[3] = 30;    // Position 0
        //        bst[1] = 10;    // Position 1
        //        bst[5] = 50;    // Position 2
        //        bst[7] = 70;    // Position 3
        //        bst[6] = 60;    // Position 4 - Deleted
        //        bst[8] = 80;    // Position 5
        //        bst[9] = 90;    // Position 6
        //        bst[10] = 100;  // Position 7
        bst.delete_node(6);
        ASSERT_EQ(7, bst.size());

        /*
         *               7B
         *             /    \
         *           3R      9R
         *          /  \    /   \
         *         1B  5B 8B    10B
         */

        ASSERT_EQ(2, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(3, 30, 3, 1, 2, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(5, 50, 0, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
        ASSERT_TRUE(are_equal(make_node(7, 70, NULL_INDEX, 0, 6, BLACK), bst.node_at(3)));
        ASSERT_TRUE(are_equal(make_node(8, 80, 6, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(5)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 3, 5, 7, RED), bst.node_at(6)));
        ASSERT_TRUE(
            are_equal(make_node(10, 100, 6, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(7)));
    }
}

TEST(FixedRedBlackTree, Deletion_CornerCases)
{
    // Delete root as the last element
    {
        FixedRedBlackTree<int, int, 20> bst{};
        ASSERT_EQ(0, bst.size());
        ASSERT_EQ(NULL_INDEX, bst.root_index());
        bst[5] = 50;  // Position 0
        ASSERT_EQ(1, bst.size());

        /*
         *               5B
         */
        ASSERT_EQ(0, find_height(bst));
        ASSERT_TRUE(
            are_equal(make_node(5, 50, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(0)));

        bst.delete_node(5);
        ASSERT_EQ(0, bst.size());
        ASSERT_EQ(0, find_height(bst));
        ASSERT_EQ(NULL_INDEX, bst.root_index());
    }

    // Delete root while it only has a left child
    {
        FixedRedBlackTree<int, int, 20> bst{};
        bst[5] = 50;  // Position 0
        bst[1] = 10;  // Position 1
        ASSERT_EQ(2, bst.size());

        /*
         *               5B
         *             /
         *           1R
         */

        ASSERT_EQ(1, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(5, 50, NULL_INDEX, 1, NULL_INDEX, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));

        bst.delete_node(5);
        ASSERT_EQ(1, bst.size());
        ASSERT_EQ(0, find_height(bst));
        ASSERT_EQ(1, bst.root_index());
        ASSERT_TRUE(
            are_equal(make_node(1, 10, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
    }

    // Delete root while it only has a right child
    {
        FixedRedBlackTree<int, int, 20> bst{};
        bst[5] = 50;  // Position 0
        bst[9] = 90;  // Position 1
        ASSERT_EQ(2, bst.size());

        /*
         *               5B
         *                 \
         *                  9R
         */

        ASSERT_EQ(1, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(5, 50, NULL_INDEX, NULL_INDEX, 1, BLACK), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(9, 90, 0, NULL_INDEX, NULL_INDEX, RED), bst.node_at(1)));

        bst.delete_node(5);
        ASSERT_EQ(1, bst.size());
        ASSERT_EQ(0, find_height(bst));
        ASSERT_EQ(1, bst.root_index());
        ASSERT_TRUE(
            are_equal(make_node(9, 90, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(1)));
    }

    // Delete root that is not in position 0 of the array while it only has a left child
    {
        FixedRedBlackTree<int, int, 20> bst{};
        bst[5] = 50;  // Position 0
        bst[3] = 30;  // Position 1
        bst[1] = 10;  // Position 2
        ASSERT_EQ(3, bst.size());

        /*
         *               3B
         *             /   \
         *           1R     5R
         */
        ASSERT_EQ(1, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(5, 50, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 2, 0, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));

        bst.delete_node(5);
        ASSERT_EQ(2, bst.size());
        ASSERT_EQ(1, find_height(bst));
        ASSERT_EQ(1, bst.root_index());
        ASSERT_TRUE(are_equal(make_node(3, 30, NULL_INDEX, 2, NULL_INDEX, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(1, 10, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));

        /*
         *               3B
         *             /
         *           1R
         */

        bst.delete_node(3);
        ASSERT_EQ(1, bst.size());
        ASSERT_EQ(0, find_height(bst));
        ASSERT_EQ(2, bst.root_index());
        ASSERT_TRUE(
            are_equal(make_node(1, 10, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK), bst.node_at(2)));
    }

    // Delete root that is not in position 0 of the array while it only has a right child
    {
        FixedRedBlackTree<int, int, 20> bst{};
        bst[5] = 50;    // Position 0
        bst[9] = 90;    // Position 1
        bst[13] = 130;  // Position 2
        ASSERT_EQ(3, bst.size());

        /*
         *               9B
         *             /   \
         *           5R     13R
         */

        ASSERT_EQ(1, find_height(bst));
        ASSERT_TRUE(are_equal(make_node(5, 50, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(0)));
        ASSERT_TRUE(are_equal(make_node(9, 90, NULL_INDEX, 0, 2, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));

        bst.delete_node(5);
        ASSERT_EQ(2, bst.size());
        ASSERT_EQ(1, find_height(bst));
        ASSERT_EQ(1, bst.root_index());
        ASSERT_TRUE(are_equal(make_node(9, 90, NULL_INDEX, NULL_INDEX, 2, BLACK), bst.node_at(1)));
        ASSERT_TRUE(are_equal(make_node(13, 130, 1, NULL_INDEX, NULL_INDEX, RED), bst.node_at(2)));

        /*
         *               9B
         *                 \
         *                  13R
         */

        bst.delete_node(9);
        ASSERT_EQ(1, bst.size());
        ASSERT_EQ(0, find_height(bst));
        ASSERT_EQ(2, bst.root_index());
        ASSERT_TRUE(are_equal(make_node(13, 130, NULL_INDEX, NULL_INDEX, NULL_INDEX, BLACK),
                              bst.node_at(2)));
    }
}

TEST(FixedRedBlackTree, IndexOfMin)
{
    FixedRedBlackTree<int, int, 10> bst{};
    bst[0] = 10;
    bst[1] = 11;
    bst[2] = 12;
    bst[3] = 13;

    bst.delete_node(0);

    ASSERT_EQ(1, bst.node_at(bst.index_of_min_at()).key());
}

TEST(FixedRedBlackTree, IndexOfMax)
{
    FixedRedBlackTree<int, int, 10> bst{};
    bst[0] = 10;
    bst[1] = 11;
    bst[2] = 12;
    bst[3] = 13;

    bst.delete_node(0);

    ASSERT_EQ(3, bst.node_at(bst.index_of_max_at()).key());
}

TEST(FixedRedBlackTree, IndexOfSuccessor)
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[5] = 50;    // Position 0
    bst[9] = 90;    // Position 1
    bst[13] = 130;  // Position 2
    ASSERT_EQ(3, bst.size());

    /*
     *               9B
     *             /   \
     *           5R     13R
     */

    ASSERT_EQ(1, bst.index_of_successor_at(0));
    ASSERT_EQ(2, bst.index_of_successor_at(1));
    ASSERT_EQ(NULL_INDEX, bst.index_of_successor_at(2));
}

TEST(FixedRedBlackTree, IndexOfPredecessor)
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[5] = 50;    // Position 0
    bst[9] = 90;    // Position 1
    bst[13] = 130;  // Position 2
    ASSERT_EQ(3, bst.size());

    /*
     *               9B
     *             /   \
     *           5R     13R
     */

    ASSERT_EQ(NULL_INDEX, bst.index_of_predecessor_at(0));
    ASSERT_EQ(0, bst.index_of_predecessor_at(1));
    ASSERT_EQ(1, bst.index_of_predecessor_at(2));
}

TEST(FixedRedBlackTree, IndexOfEntryLower)
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[5] = 50;    // Position 0
    bst[9] = 90;    // Position 1
    bst[13] = 130;  // Position 2
    ASSERT_EQ(3, bst.size());

    /*
     *               9B
     *             /   \
     *           5R     13R
     */

    ASSERT_EQ(NULL_INDEX, bst.index_of_node_lower(4));
    ASSERT_EQ(NULL_INDEX, bst.index_of_node_lower(5));
    ASSERT_EQ(0, bst.index_of_node_lower(7));
    ASSERT_EQ(0, bst.index_of_node_lower(9));
    ASSERT_EQ(1, bst.index_of_node_lower(12));
    ASSERT_EQ(1, bst.index_of_node_lower(13));
    ASSERT_EQ(2, bst.index_of_node_lower(14));
}

TEST(FixedRedBlackTree, IndexOfEntryHigher)
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[5] = 50;    // Position 0
    bst[9] = 90;    // Position 1
    bst[13] = 130;  // Position 2
    ASSERT_EQ(3, bst.size());

    /*
     *               9B
     *             /   \
     *           5R     13R
     */

    ASSERT_EQ(0, bst.index_of_node_higher(4));
    ASSERT_EQ(1, bst.index_of_node_higher(5));
    ASSERT_EQ(1, bst.index_of_node_higher(7));
    ASSERT_EQ(2, bst.index_of_node_higher(9));
    ASSERT_EQ(2, bst.index_of_node_higher(12));
    ASSERT_EQ(NULL_INDEX, bst.index_of_node_higher(13));
    ASSERT_EQ(NULL_INDEX, bst.index_of_node_higher(14));
}

TEST(FixedRedBlackTree, IndexOfEntryFloor)
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[5] = 50;    // Position 0
    bst[9] = 90;    // Position 1
    bst[13] = 130;  // Position 2
    ASSERT_EQ(3, bst.size());

    /*
     *               9B
     *             /   \
     *           5R     13R
     */

    ASSERT_EQ(NULL_INDEX, bst.index_of_node_lower(4));
    ASSERT_EQ(0, bst.index_of_node_floor(5));
    ASSERT_EQ(0, bst.index_of_node_floor(7));
    ASSERT_EQ(1, bst.index_of_node_floor(9));
    ASSERT_EQ(1, bst.index_of_node_floor(12));
    ASSERT_EQ(2, bst.index_of_node_floor(13));
    ASSERT_EQ(2, bst.index_of_node_floor(14));
}

TEST(FixedRedBlackTree, IndexOfEntryCeiling)
{
    FixedRedBlackTree<int, int, 20> bst{};
    bst[5] = 50;    // Position 0
    bst[9] = 90;    // Position 1
    bst[13] = 130;  // Position 2
    ASSERT_EQ(3, bst.size());

    /*
     *               9B
     *             /   \
     *           5R     13R
     */

    ASSERT_EQ(0, bst.index_of_node_ceiling(4));
    ASSERT_EQ(0, bst.index_of_node_ceiling(5));
    ASSERT_EQ(1, bst.index_of_node_ceiling(7));
    ASSERT_EQ(1, bst.index_of_node_ceiling(9));
    ASSERT_EQ(2, bst.index_of_node_ceiling(12));
    ASSERT_EQ(2, bst.index_of_node_ceiling(13));
    ASSERT_EQ(NULL_INDEX, bst.index_of_node_ceiling(14));
}

template <std::size_t MAXIMUM_SIZE>
static void consistency_test_helper(const std::array<int, MAXIMUM_SIZE>& insertion_order,
                                    const std::array<int, MAXIMUM_SIZE>& deletion_order,
                                    FixedRedBlackTree<int, int, MAXIMUM_SIZE>& bst)
{
    static constexpr std::size_t HALF_MAXIMUM_SIZE = MAXIMUM_SIZE / 2;
    static constexpr std::size_t QUARTER_MAXIMUM_SIZE = MAXIMUM_SIZE / 4;

    // Insert all and verify elements as we go
    for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
    {
        ASSERT_TRUE(contains_all_from_to(bst, insertion_order, 0, i));
        bst[insertion_order[i]] = insertion_order[i];
    }
    ASSERT_TRUE(contains_all_from_to(bst, insertion_order, 0, MAXIMUM_SIZE));

    // Remove all and verify elements as we go
    for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
    {
        ASSERT_TRUE(contains_all_from_to(bst, deletion_order, i, MAXIMUM_SIZE));
        const int value_to_delete = deletion_order[i];

        // Copy the value, as the node might move.
        const int expected_successor_value = [&]()
        {
            // gt will be invalid after the deletion, so hide it with scope
            const NodeIndex gt = bst.index_of_node_higher(value_to_delete);
            return bst.contains_at(gt) ? bst.node_at(gt).value() : 0;
        }();

        const NodeIndex index_to_delete = bst.index_of_node_or_null(value_to_delete);
        const NodeIndex successor_index = bst.delete_at_and_return_successor(index_to_delete);

        const int actual_successor_value =
            bst.contains_at(successor_index) ? bst.node_at(successor_index).value() : 0;
        ASSERT_EQ(expected_successor_value == 0, successor_index == NULL_INDEX);
        ASSERT_EQ(expected_successor_value, actual_successor_value);
    }
    ASSERT_TRUE(bst.empty());

    // Mix insertions and deletions
    for (std::size_t i = 0; i < HALF_MAXIMUM_SIZE; i++)
    {
        ASSERT_TRUE(contains_all_from_to(bst, insertion_order, 0, i));
        bst[insertion_order[i]] = insertion_order[i];
    }
    for (std::size_t i = 0; i < QUARTER_MAXIMUM_SIZE; i++)
    {
        ASSERT_TRUE(contains_all_from_to(
            bst, insertion_order, QUARTER_MAXIMUM_SIZE, QUARTER_MAXIMUM_SIZE + i));
        bst.delete_node(insertion_order[i]);
    }
    ASSERT_TRUE(
        contains_all_from_to(bst, insertion_order, QUARTER_MAXIMUM_SIZE, HALF_MAXIMUM_SIZE));
    for (std::size_t i = 0; i < QUARTER_MAXIMUM_SIZE; i++)
    {
        ASSERT_TRUE(contains_all_from_to(bst, insertion_order, 0, i));
        ASSERT_TRUE(
            contains_all_from_to(bst, insertion_order, QUARTER_MAXIMUM_SIZE, HALF_MAXIMUM_SIZE));
        bst[insertion_order[i]] = insertion_order[i];
    }
    ASSERT_TRUE(contains_all_from_to(bst, insertion_order, 0, HALF_MAXIMUM_SIZE));
    for (std::size_t i = 0; i < HALF_MAXIMUM_SIZE; i++)
    {
        ASSERT_TRUE(contains_all_from_to(bst, insertion_order, i, HALF_MAXIMUM_SIZE));
        bst.delete_node(insertion_order[i]);
    }
}

TEST(FixedRedBlackTree, ConsistencyRegressionTest1)
{
    static constexpr std::size_t MAXIMUM_SIZE = 8;

    // Intentionally use the same bst for this entire test. Don't clear()
    FixedRedBlackTree<int, int, MAXIMUM_SIZE> bst{};

    std::array<int, MAXIMUM_SIZE> insertion_order{2, 4, 3, 6, 1, 5, 0, 7};
    std::array<int, MAXIMUM_SIZE> deletion_order{3, 4, 1, 2, 6, 0, 5, 7};

    consistency_test_helper(insertion_order, deletion_order, bst);
}

TEST(FixedRedBlackTree, RandomizedConsistencyTest)
{
    static constexpr std::size_t MAXIMUM_SIZE = 8;
    // Intentionally use the same bst for this entire test. Don't clear()
    FixedRedBlackTree<int, int, MAXIMUM_SIZE> bst{};

    std::array<int, MAXIMUM_SIZE> insertion_order{};
    std::array<int, MAXIMUM_SIZE> deletion_order{};

    for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
    {
        insertion_order[i] = static_cast<int>(i);
        deletion_order[i] = static_cast<int>(i);
    }

    static constexpr std::size_t ITERATIONS = 20;
    std::random_device rd;
    std::mt19937 g(rd());
    for (std::size_t iteration = 0; iteration < ITERATIONS; iteration++)
    {
        std::shuffle(insertion_order.begin(), insertion_order.end(), g);
        std::shuffle(deletion_order.begin(), deletion_order.end(), g);
        consistency_test_helper(insertion_order, deletion_order, bst);
    }
}

TEST(FixedRedBlackTree, TreeMaxHeight)
{
    static constexpr std::size_t MAXIMUM_SIZE = 512;
    FixedRedBlackTree<int, int, MAXIMUM_SIZE> bst{};

    std::array<int, MAXIMUM_SIZE> insertion_order{};
    for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
    {
        insertion_order[i] = static_cast<int>(i);
    }

    // Ascending Insertion
    for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
    {
        bst[insertion_order[i]] = insertion_order[i];
        ASSERT_LE(find_height(bst), max_height_of_red_black_tree(bst.size()));
    }

    // Descending Insertion
    std::reverse(insertion_order.begin(), insertion_order.end());
    for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
    {
        bst[insertion_order[i]] = insertion_order[i];
        ASSERT_LE(find_height(bst), max_height_of_red_black_tree(bst.size()));
    }

    // Randomized Insertion
    static constexpr std::size_t ITERATIONS = 10;
    std::random_device rd;
    std::mt19937 g(rd());
    for (std::size_t iteration = 0; iteration < ITERATIONS; iteration++)
    {
        std::shuffle(insertion_order.begin(), insertion_order.end(), g);
        for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
        {
            bst[insertion_order[i]] = insertion_order[i];
            ASSERT_LE(find_height(bst), max_height_of_red_black_tree(bst.size()));
        }
    }
}
}  // namespace fixed_containers::fixed_red_black_tree_detail
