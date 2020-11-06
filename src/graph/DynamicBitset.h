#ifndef WEIGHTED_F_FREE_EDGE_EDITING_DYNAMICBITSET_H
#define WEIGHTED_F_FREE_EDGE_EDITING_DYNAMICBITSET_H

#include <vector>
#include <limits>
#include <cassert>

namespace dynamic_bitset {

    // Forward declaration
    template<typename Block>
    class IndexIterator;


    namespace detail {
        template<typename T>
        [[nodiscard]] constexpr int popcount(T n) noexcept {
            static_assert(std::is_unsigned_v<T>);
            int c = 0;
            for (; n; ++c)
                n &= n - 1;
            return c;
        }

        template<>
        [[nodiscard]] constexpr int popcount<unsigned>(unsigned n) noexcept {
            return __builtin_popcount(n);
        }

        template<>
        [[nodiscard]] constexpr int popcount<unsigned long>(unsigned long n) noexcept {
            return __builtin_popcountl(n);
        }

        template<>
        [[nodiscard]] constexpr int popcount<unsigned long long>(unsigned long long n) noexcept {
            return __builtin_popcountll(n);
        }

        template<>
        [[nodiscard]] constexpr int popcount<unsigned short>(unsigned short n) noexcept {
            return popcount(static_cast<unsigned>(n));
        }


        template<typename T>
        [[nodiscard]] constexpr int ctz(T n) noexcept {
            static_assert(std::is_unsigned_v<T>);
            if (!n)
                return std::numeric_limits<T>::digits;
            int c = 0;
            while (!(n & 1))
                ++c;
            return c;
        }

        template<>
        [[nodiscard]] constexpr int ctz<unsigned>(unsigned n) noexcept {
            return __builtin_ctz(n);
        }

        template<>
        [[nodiscard]] constexpr int ctz<unsigned short>(unsigned short n) noexcept {
            if (!n)
                return std::numeric_limits<unsigned short>::digits;
            return ctz(static_cast<unsigned>(n));
        }

        template<>
        [[nodiscard]] constexpr int ctz<unsigned long>(unsigned long n) noexcept {
            return __builtin_ctzl(n);
        }

        template<>
        [[nodiscard]] constexpr int ctz<unsigned long long>(unsigned long long n) noexcept {
            return __builtin_ctzll(n);
        }

        template<typename T, typename = std::enable_if_t<std::is_signed_v<T>>>
        [[nodiscard]] constexpr int ffs(T n) noexcept {
            if (!n)
                return 0;
            return ctz(static_cast<std::make_unsigned<T>>(n)) + 1;
        }

        template<>
        [[nodiscard]] constexpr int ffs<int>(int n) noexcept {
            return __builtin_ffs(n);
        }

        template<>
        [[nodiscard]] constexpr int ffs<long>(long n) noexcept {
            return __builtin_ffsl(n);
        }

        template<>
        [[nodiscard]] constexpr int ffs<long long>(long long n) noexcept {
            return __builtin_ffsll(n);
        }
    }

    template<typename Block = unsigned long long>
    class DynamicBitset {
        static_assert(std::is_unsigned_v<Block>);
        /**
         * This class is in large parts taken from boost::dynamic_bitset<>.
         *
         * This was done to have another method of iterating the set bits other than find_first and find_next.
         */

        /** License of boost::dynamic_bitset<>
        Boost Software License - Version 1.0 - August 17th, 2003

        Permission is hereby granted, free of charge, to any person or organization
        obtaining a copy of the software and accompanying documentation covered by
        this license (the "Software") to use, reproduce, display, distribute,
        execute, and transmit the Software, and to prepare derivative works of the
        Software, and to permit third-parties to whom the Software is furnished to
        do so, all subject to the following:

        The copyright notices in the Software and this entire statement, including
        the above license grant, this restriction and the following disclaimer,
        must be included in all copies of the Software, in whole or in part, and
        all derivative works of the Software, unless such copies or derivative
        works are solely in the form of machine-executable object code generated by
        a source language processor.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
        SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
        FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
        ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
        DEALINGS IN THE SOFTWARE.
         */

    public:
        using block_type = Block;
        using size_type = std::size_t;
        using block_width_type = typename std::vector<Block>::size_type;

        static constexpr block_width_type bits_per_block = std::numeric_limits<Block>::digits;
        static constexpr size_type npos = static_cast<size_type>(-1);

    private:
        size_type m_num_bits;
        std::vector<Block> m_blocks;

    public:

        class reference {
        private:
            block_type &m_block;
            const block_type m_mask;

            friend class DynamicBitset<Block>;

            constexpr reference(block_type &b, block_width_type pos) noexcept:
                    m_block(b), m_mask(block_type(1) << pos) {
                assert(pos < bits_per_block);
            }

            // NOLINTNEXTLINE
            void operator&() = delete;

        public:

            [[nodiscard]] constexpr operator bool() const noexcept { // NOLINT(google-explicit-constructor)
                return (m_block & m_mask) != 0;
            }

            [[nodiscard]] constexpr bool operator~() const noexcept {
                return (m_block & m_mask) == 0;
            }

            constexpr reference &flip() noexcept {
                m_block ^= m_mask;
                return *this;
            }

            constexpr reference &operator=(bool x) noexcept {
                if (x) {
                    m_block |= m_mask;
                } else {
                    m_block &= ~m_mask;
                }
                return *this;
            }

            constexpr reference &operator=(const reference &rhs) noexcept {
                *this = bool(rhs);
                return *this;
            }

            constexpr reference &operator|=(bool x) noexcept {
                if (x) {
                    m_block |= m_mask;
                }
                return *this;
            }

            constexpr reference &operator&=(bool x) noexcept {
                if (!x) {
                    m_block &= ~m_mask;
                }
                return *this;
            }

            constexpr reference &operator^=(bool x) noexcept {
                if (x) {
                    m_block ^= m_mask;
                }
                return *this;
            }

            constexpr reference &operator-=(bool x) noexcept {
                if (x) {
                    m_block &= ~m_mask;
                }
                return *this;
            }
        };

        using const_reference = bool;

        constexpr DynamicBitset() noexcept: m_num_bits(0) {}

        explicit DynamicBitset(size_type num_bits) :
                m_num_bits(num_bits), m_blocks(num_bits ? block_index(num_bits - 1) + 1 : 0) {}

        DynamicBitset(const DynamicBitset &b) = default;

        ~DynamicBitset() {
            assert(check_invariants());
        };

        void swap(DynamicBitset &other) {
            using std::swap;
            swap(m_blocks, other.m_blocks);
            swap(m_num_bits, other.m_num_bits);
        }

        DynamicBitset &operator=(const DynamicBitset &b) = default;

        DynamicBitset(DynamicBitset &&src) noexcept = default;

        DynamicBitset &operator=(DynamicBitset &&src) noexcept = default;

        void clear() {
            m_blocks.clear();
            m_num_bits = 0;
        }

        DynamicBitset &operator&=(const DynamicBitset &rhs) {
            assert(size() == rhs.size());
            for (size_type i = 0; i < num_blocks(); ++i)
                m_blocks[i] &= rhs.m_blocks[i];
            return *this;
        }

        DynamicBitset &operator|=(const DynamicBitset &rhs) {
            assert(size() == rhs.size());
            for (size_type i = 0; i < num_blocks(); ++i)
                m_blocks[i] |= rhs.m_blocks[i];
            return *this;
        }

        DynamicBitset &operator^=(const DynamicBitset &rhs) {
            assert(size() == rhs.size());
            for (size_type i = 0; i < num_blocks(); ++i)
                m_blocks[i] ^= rhs.m_blocks[i];
            return *this;
        }

        DynamicBitset &operator-=(const DynamicBitset &rhs) {
            assert(size() == rhs.size());
            for (size_type i = 0; i < num_blocks(); ++i)
                m_blocks[i] &= ~rhs.m_blocks[i];
            return *this;
        }


        DynamicBitset &set() {
            std::fill(m_blocks.begin(), m_blocks.end(), ~static_cast<Block>(0));
            zero_unused_bits();
            return *this;
        }

        DynamicBitset &set(size_type pos) {
            assert(pos < m_num_bits);
            m_blocks[block_index(pos)] |= bit_mask(pos);
            return *this;
        }

        DynamicBitset &reset() {
            std::fill(m_blocks.begin(), m_blocks.end(), Block(0));
            return *this;
        }

        DynamicBitset &reset(size_type pos) {
            assert(pos < m_num_bits);
            m_blocks[block_index(pos)] &= ~bit_mask(pos);
            return *this;
        }

        DynamicBitset &flip() {
            for (size_type i = 0; i < num_blocks(); ++i)
                m_blocks[i] = ~m_blocks[i];
            zero_unused_bits();
            return *this;
        }

        DynamicBitset &flip(size_type pos) {
            assert(pos < m_num_bits);
            m_blocks[block_index(pos)] ^= bit_mask(pos);
            return *this;
        }

        [[nodiscard]] bool test(size_type pos) const {
            assert(pos < m_num_bits);
            return (m_blocks[block_index(pos)] & bit_mask(pos)) != 0;
        }

        [[nodiscard]] reference operator[](size_type pos) {
            assert(pos < m_num_bits);
            return reference(m_blocks[block_index(pos)], bit_index(pos));
        }

        [[nodiscard]] const_reference operator[](size_type pos) const {
            assert(pos < m_num_bits);
            return test(pos);
        }

        [[nodiscard]] constexpr size_type size() const noexcept {
            return m_num_bits;
        }

        [[nodiscard]] constexpr size_type num_blocks() const noexcept {
            return m_blocks.size();
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return size() == 0;
        }

        [[nodiscard]] size_type count() const noexcept {
            size_type count = 0;
            for (size_type i = 0; i < num_blocks(); ++i)
                count += static_cast<size_type>(detail::popcount(m_blocks[i]));
            return count;
        }

        [[nodiscard]] size_type find_first() const noexcept {
            return find_from(0);
        }

        [[nodiscard]] size_type find_next(size_type pos) const noexcept {
            if (pos >= (size() - 1) || size() == 0)
                return npos;

            ++pos;

            const auto blk = block_index(pos);
            const auto idx = bit_index(pos);

            const Block fore = m_blocks[blk] >> idx;

            return fore
                   ? pos + static_cast<size_type>(detail::ctz(fore))
                   : find_from(blk + 1);
        }

    private:
        [[nodiscard]] size_type find_from(size_type first_block) const noexcept {
            size_type i = first_block;
            while (i < m_blocks.size() && !m_blocks[i])
                ++i;

            if (i >= m_blocks.size())
                return npos;

            return i * bits_per_block + static_cast<size_type>(detail::ctz(m_blocks[i]));
        }

        void zero_unused_bits() {
            assert (num_blocks() == calc_num_blocks(m_num_bits));

            // if != 0 this is the number of bits used in the last block
            const block_width_type extra_bits = count_extra_bits();

            if (extra_bits != 0)
                highest_block() &= (Block(1) << extra_bits) - 1;
        }

        [[nodiscard]] bool check_invariants() const {
            const block_width_type extra_bits = count_extra_bits();
            if (extra_bits > 0) {
                const block_type mask = (~block_type(0)) << extra_bits;
                if ((highest_block() & mask) != 0)
                    return false;
            }
            if (m_blocks.size() > m_blocks.capacity() || num_blocks() != calc_num_blocks(size()))
                return false;

            return true;
        };

        [[nodiscard]] constexpr block_width_type count_extra_bits() const noexcept {
            return bit_index(size());
        }

        [[nodiscard]] constexpr static size_type block_index(size_type pos) noexcept {
            return pos / bits_per_block;
        }

        [[nodiscard]] constexpr static block_width_type bit_index(size_type pos) noexcept {
            return static_cast<block_width_type>(pos % bits_per_block);
        }

        [[nodiscard]] constexpr static Block bit_mask(size_type pos) noexcept {
            return Block(1) << bit_index(pos);
        }

        [[nodiscard]] constexpr static size_type calc_num_blocks(size_type num_bits) {
            return num_bits / bits_per_block
                   + static_cast<size_type>( num_bits % bits_per_block != 0 );
        }

        [[nodiscard]] Block &highest_block() {
            assert(size() > 0 && num_blocks() > 0);
            return m_blocks.back();
        }

        [[nodiscard]] const Block &highest_block() const {
            assert(size() > 0 && num_blocks() > 0);
            return m_blocks.back();
        }

        friend class IndexIterator<Block>;
    };


    template<typename Block>
    inline void swap(DynamicBitset<Block> &left, DynamicBitset<Block> &right) noexcept {
        left.swap(right);
    }


    template<typename T>
    class IndexIterator {
    private:
        using Bitset = DynamicBitset<T>;
        using Block = T;
    public:
        using size_type = typename Bitset::size_type;

    private:
        const Bitset *m_bitset{nullptr};
        size_type m_pos{};

    public:
        using value_type = typename Bitset::size_type;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = value_type;
        using iterator_category = std::forward_iterator_tag;

        constexpr IndexIterator() noexcept = default;

        [[maybe_unused]] constexpr explicit IndexIterator(const Bitset &bitset) noexcept:
                m_bitset(std::addressof(bitset)) {
            const auto &blocks = m_bitset->m_blocks;
            const auto num_blocks = blocks.size();
            const auto num_bits = m_bitset->size();

            size_type block_idx = 0;
            while (block_idx < num_blocks && (blocks[block_idx] == 0))
                ++block_idx;

            m_pos = block_idx >= num_blocks
                    ? num_bits
                    : block_idx * Bitset::bits_per_block + static_cast<size_type>(detail::ctz(blocks[block_idx]));
            assert(m_pos <= num_bits);
        };

        constexpr IndexIterator(const Bitset &bitset, size_type pos) noexcept:
                m_bitset(std::addressof(bitset)), m_pos(pos) {
            assert(m_pos <= m_bitset->size());
        };

        [[nodiscard]] constexpr value_type operator*() const noexcept {
            assert(m_pos < m_bitset->size());
            return m_pos;
        }

        IndexIterator &operator++() noexcept {
            assert(m_pos < m_bitset->size());
            const auto &blocks = m_bitset->m_blocks;
            const auto num_blocks = blocks.size();
            const auto num_bits = m_bitset->size();

            ++m_pos;
            if (m_pos == num_bits)
                return *this;

            auto block_idx = Bitset::block_index(m_pos);
            const auto idx = Bitset::bit_index(m_pos);

            assert(block_idx < num_blocks);
            const auto current_block_rest = blocks[block_idx] >> idx;
            if (current_block_rest != 0) {
                m_pos += static_cast<size_type>(detail::ctz(current_block_rest));
                assert(m_pos <= num_bits);
                return *this;
            }

            ++block_idx;
            while (block_idx < num_blocks && (blocks[block_idx] == 0))
                ++block_idx;

            m_pos = block_idx >= num_blocks
                    ? num_bits
                    : block_idx * Bitset::bits_per_block + static_cast<size_type>(detail::ctz(blocks[block_idx]));

            assert(m_pos <= num_bits);
            return *this;
        }

        [[nodiscard]] IndexIterator operator++(int) noexcept { // NOLINT(cert-dcl21-cpp)
            IndexIterator copy(*this);
            ++*this;
            return copy;
        }

        [[nodiscard]] constexpr bool operator==(const IndexIterator &other) const noexcept {
            return m_pos == other.m_pos;
        }

        [[nodiscard]] constexpr bool operator!=(const IndexIterator &other) const noexcept {
            return !(*this == other);
        }

        [[nodiscard]] constexpr bool operator<(const IndexIterator &other) const noexcept {
            return m_pos < other.m_pos;
        }

        [[nodiscard]] constexpr bool operator<=(const IndexIterator &other) const noexcept {
            return m_pos <= other.m_pos;
        }

        [[nodiscard]] constexpr bool operator>(const IndexIterator &other) const noexcept {
            return m_pos > other.m_pos;
        }

        [[nodiscard]] constexpr bool operator>=(const IndexIterator &other) const noexcept {
            return m_pos >= other.m_pos;
        }
    };

}

#endif //WEIGHTED_F_FREE_EDGE_EDITING_DYNAMICBITSET_H
