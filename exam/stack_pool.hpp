/**
 * @file stack_pool.hpp
 * @author Carlo De Nardin
 * @brief A pool for fast stack. A stack is a data structure that is based on
 * the LIFO principle (Last In First Out).
 *
 * This programs implements a pool (stack_pool) of multiple fast stack. 
 * The stack_pool stores each node in a std::vector. The address of a node is 
 * 1 + idx, where idx is the index where the node is stored in the vector. 
 * This rappresentation allows to use the address 0 as end. The first node 
 * stored in the vector will be put at idx == 0 but it will references as 1.
 *
 * The stack_pool maintains a stack of free nodes for the available positions in
 * the vector. The free nodes is empty at the beggining.
 *  
 * Operations that can be performed on a stack:
 * - push: insert an element in the head of the stack;
 * - pop: remove the element that is in the head of the stack.
 * 
 * @version 1.0
 * @date 2022-02-12
 */

#include <iostream>
#include <vector>
#include <exception>

/**
 * @brief EmptyStackException handle the event in which a user tries to pop
 * an empty stack.
 * 
 */
struct EmptyStackException : public std::exception {
    std::string message;
    EmptyStackException(std::string message) : message {message} {}
};

/**
 * @brief NotStackHeadException handle the event in which a user tries to pop
 * a stack by passing a no-head element.
 * 
 */
struct NotStackHeadException : public std::exception {
    std::string message;
    NotStackHeadException(std::string message) : message {message} {}
};

/**
 * @brief NotEqualTypeException handle the event in which a user tries to insert
 * a different type value in a stack of the pool. A pool has a specific type
 * value. With this exception we can handle and avoid a possible implicit 
 * conversion.
 * 
 */
struct NotEqualTypeException : public std::exception {
    std::string message;
    NotEqualTypeException(std::string message) : message {message} {}
};

/**
 * @brief RangeCheckingException handle the event in which a user tries to
 * access a portion of memory that does not belong to the pool.
 * 
 */
 struct RangeCheckingException : public std::exception {
    std::string message;
    RangeCheckingException(std::string message) : message {message} {}
 };

/**
 * @brief Custom iterator for iterating among the pool stack.
 * 
 * @tparam P pool_stack.
 * @tparam T value_type Type of the value.
 * @tparam N stack_type Type of the stack head (Default value is setted
 * in the stack_pool template equals to size_t).
 */
template <typename P, typename T, typename N>
class _iterator {
    private:
        P* pool;
        N current;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        _iterator(P* const pool, N current) : pool{pool}, current{current} {}

        /**
         * @brief Dereference operator.
         * 
         */
        reference operator* () const { 
            return pool -> value(current); 
        }

        /**
         * @brief Pre-increment.
         * 
         */
        _iterator& operator++ () { 
            current = pool -> next(current); 
            return *this; 
        }

        /**
         * @brief Post-increment.
         * 
         */
        _iterator& operator++ (int) {
            auto t {*this};
            ++(*this);
            return t;
        }

        /**
         * @brief Equality.
         *
         */
        friend bool operator== (const _iterator& a, const _iterator& b) { 
            return a.current == b.current; 
        }

        /**
         * @brief Not equality.
         * 
         */
        friend bool operator!= (const _iterator& a, const _iterator& b) { 
            return !(a == b);
        }   
};

/**
 * @brief Stack_pool concept that can handle multiple stack in the same vector.
 * 
 * The user who interfaces with the following implementation must correctly 
 * manage the push and the stack operations and in particular the return type
 * which corresponds to the head of the stack.
 * 
 * @tparam T value_type Type of the value.
 * @tparam N stack_type Type of the stack head (default std::size_t).
 */
template <typename T, typename N = std::size_t>
class stack_pool {
    private:
        
        /**
         * @brief implementations of the cocept node_t. It rappresent
         * an element of the pool vector.
         * Each node has a value, a "pointer" to the next node in the stack and a first 
         * variable boolean that describe if the node is the first element of the stack.
         * 
         */
        struct node_t {
            public:
                T value;
                N next;
                bool isHead;
                
                template <typename X>
                node_t(X &&x, N n, bool isHead) : value{std::forward<X>(x)}, next{n}, isHead{isHead} {}
        };

        using stack_type = N;
        using value_type = T;
        using size_type = typename std::vector<node_t>::size_type;
        
        std::vector<node_t> pool;
        stack_type free_nodes;

        node_t& node (stack_type x) { 
            if (x > capacity()) {
                throw RangeCheckingException(
                    "You tried to access a portion of memory that does not belong to you."
                );
            }

            return pool[x - 1]; 
        }

        const node_t& node (stack_type x) const { 
            if (x > capacity()) {
                throw RangeCheckingException(
                    "You tried to access a portion of memory that does not belong to you."
                );
            }

            return pool[x - 1]; 
        }

        /**
         * @brief Insert a new value on the top of a specific stack. The new
         * value will be the new head of the stack.
         * 
         * Exception:
         * - NotEqualTypeException if a different type from the type_value is
         * passed.
         *
         * @param val Value to insert in the stack. (R-value reference).
         * @param head Head of the stack in which the value must be inserted.
         * @return stack_type New head of the stack.
         */
        template <typename X>
        stack_type _push (X&& val, stack_type head) {
            if (!std::is_same<X, value_type>::value){
                throw NotEqualTypeException(
                    "You tried to push in the stack an incorrect type."
                );
            }

            if (!empty(free_nodes)) {
                auto t = free_nodes;
                value(free_nodes) = val;
                free_nodes = next(free_nodes);
                next(t) = head;
                isHead(head) = false;
                isHead(t) = true;
                return t;
            } else {
                pool.emplace_back(std::forward<X>(val), head, true);
                isHead(head) = false;
                return static_cast<stack_type>(pool.size());
            }   
        }
    
    public:

        /**
         * @brief Construct a new stack pool object which capacity is 0.
         * 
         */
        stack_pool() noexcept : free_nodes{end()} {};

        /**
         * @brief Construct a new stack pool object which capacity is n.
         * 
         * Exception: (inherited from **std::vector::reserve** method)
         * - std::length_error if n > max_size().
         * - std::bad_alloc by Allocator::allocate().
         *
         * @param n Initial capacity.
         */
        explicit stack_pool(size_type n) : free_nodes{end()} {
            reserve(n);
        }

        using iterator = _iterator<stack_pool, T, N>;
        using const_iterator = _iterator<stack_pool, const T, N>;

        iterator begin(stack_type x) noexcept { return iterator(this, x); }
        iterator end(stack_type ) noexcept { return iterator(this, end()); }

        const_iterator begin(stack_type x) const noexcept { 
            return const_iterator(this, x); 
        }
        const_iterator end(stack_type ) const noexcept { 
            return const_iterator(this, end()); 
        }

        const_iterator cbegin(stack_type x) const noexcept { 
            return const_iterator(this, x); 
        }
        const_iterator cend(stack_type ) const noexcept { 
            return const_iterator(this, end()); 
        }

        /**
         * @brief Create a new stack in the stack_pool.
         *
         * This is a read-only function that it does not modify the status of the object.
         *
         * Exception:
         * - No-throw guarantee: this member functions never throws exceptions.
         * 
         * @return stack_type Head of the new stack equals to the end.
         */
        stack_type new_stack () const noexcept {
            return end();
        }

        /**
         * @brief Increase the capacity of the vector to a value that's greater or equal to n. 
         * If n is greater than the current capacity() new storage is allocated.
         *
         * Exception:
         * - std::length_error if n > max_size()
         * - std::bad_alloc by Allocator::allocate() 
         * 
         * @param n Capacity that the user want to allocate initially .
         */
        void reserve (size_type n) {
            pool.reserve(n);
        }

        /**
         * @brief Returns the size of the storage space currently allocated for the vector, 
         * expressed in terms of elements.
         *
         * This is a read-only function that it does not modify the status of the object.
         *
         * Exception:
         * - No-throw guarantee: this member functions never throws exceptions.
         * 
         * @return size_type Pool capacity.
         */
        size_type capacity() const noexcept {
            return pool.capacity();
        }

        /**
         * @brief Check if a stack is empty.
         *
         * This is a read-only function that it does not modify the status of the object.
         *
         * Exception:
         * - No-throw guarantee: this member functions never throws exceptions.
         *
         * @param x Head of the stack.
         * @return true If the stack is empty.
         * @return false If the stack is not empty.
         */
        bool empty (stack_type x) const noexcept {
            return x == end();
        }

        /**
         * @brief Represents the common end of the stacks.
         * 
         * @return stack_type 
         */
        stack_type end() const noexcept { return stack_type(0); }

        /**
         * @brief Return the value of a node.
         *
         * Exception: (inherited from node method)
         * - RangeCheckingException if an index not in range of the pool is
         * is given. This avoid undefined behaviour and segmentation fault.
         * 
         * @param x Index of the node (usually it will the head of the stack).
         * @return T& Value of the node.
         */
        T& value(stack_type x) { return node(x).value; }
        const T& value(stack_type x) const { return node(x).value; }

        /**
         * @brief Return the index of the next node.
         *
         * Exception: (inherited from node method)
         * - RangeCheckingException if an index not in range of the pool is
         * is given. This avoid undefined behaviour and segmentation fault.
         * 
         * @param x Index of the node (usually it will the head of the stack).
         * @return T& Value of the node.
         */
        stack_type& next(stack_type x) { return node(x).next; }
        const stack_type& next(stack_type x) const { return node(x).next; }

        /**
         * @brief Return true if the node is the head of the stack.
         * 
         * Exception: (inherited from **node** method)
         * - RangeCheckingException if an index not in range of the pool is
         * is given. This avoid undefined behaviour and segmentation fault.
         * 
         * @param x Index of the node (usually it will the head of the stack).
         * @return true if the node is the head.
         * @return false if the node is not the head.
         */
        bool& isHead(stack_type x) { return node(x).isHead; }
        const bool& isHead(stack_type x) const { return node(x).isHead; }

        /**
         * @brief Insert in a specific stack a new value.
         * 
         * Exception: (inherited from **_push** method)
         * - NotEqualTypeException if a different type from the type_value is
         * passed.
         *
         * @param val Value to insert in the stack. (L-value reference).
         * @param head Head of the stack in which the value must be inserted.
         * @return stack_type New head of the stack.
         */
        stack_type push(const T& val, stack_type head) {
            return _push(val, head);
        }
        
        /**
         * @brief Insert in a specific stack a new value.
         * 
         * Exception: (inherited from **_push** method)
         * - NotEqualTypeException if a different type from the type_value is
         * passed.
         *
         * @param val Value to insert in the stack. (R-value reference).
         * @param head Head of the stack in which the value must be inserted.
         * @return stack_type New head of the stack.
         */
        stack_type push(T&& val, stack_type head) {
            return _push(std::move(val), head);
        }

        /**
         * @brief Remove the first element of a given stack.
         *
         * Exception:
         * - NotStackHeadException if a no-head is passed.
         * - EmptyStackException if an empty stack is passed.
         *
         * @param x Head of the stack.
         * @return stack_type New head of the stack which will be equal to end().
         */
        stack_type pop (stack_type x) {
            if (!isHead(x)) {
                throw NotStackHeadException(
                    "You tried to pop a stack by passing a no-head."
                );
            }

            if (empty(x)) {
                throw EmptyStackException("You tried to pop an empty stack.");
            }
            
            auto t = next(x);
            next(x) = free_nodes;
            free_nodes = x;
            value(x) = -1; // Debug info (not neccessary)
            isHead(t) = true;
            isHead(x) = false;
            return t;
        }

        /**
         * @brief Remove all the elements of a given stack.
         *
         * Exception: (inherited from pop method)
         * - NotStackHeadException if a no-head is passed
         * - EmptyStackException if an empty stack is passed
         *
         * @param x Head of the stack.
         * @return stack_type New head of the stack which will be equal to end().
         */
        stack_type free_stack (stack_type x) {
            while (x != end()) {
                x = pop(x);
            }

            return x;
        }

        /**
         * @brief Calculate the length of a given stack.
         *
         * Exception:
         * - No-throw guarantee: this member functions never throws exceptions
         *
         * @param x Head of the stack.
         * @return auto Length of the stack.
         */
        auto length (stack_type x) noexcept {
            std::size_t l {0};
            for (auto it = begin(x); it != end(x); ++it) {
                ++l;
            }
            return l;
        }

        /**
         * @brief Print a stack from the head until the end.
         *
         * Exception:
         * - No-throw guarantee: this member functions never throws exceptions
         *
         * @param x Head of the stack.
         */
        void print_stack(stack_type x) noexcept {
            for (auto it = begin(x); it != end(x); ++it) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
        }

        /**
         * @brief Print the pool.
         *
         * Useful for debug info
         *
         * Exception:
         * - No-throw guarantee: this member functions never throws exceptions
         * Const
         *
         */
        void print_pool() noexcept {
            for (auto i = 0; i < pool.size(); i++) {
                std::cout << pool[i].value << " ";
            }
            std::cout << std::endl;
        }
};

/**
 * EXAMPLE:
 *
 *  main () {
 *      stack_pool<int> pool {5};
 *    
 *      auto s1 = pool.new_stack();
 *
 *      try {
 *          s1 = pool.push(1, s1);
 *          s1 = pool.push(2, s1);
 *          s1 = pool.pop(s1);
 *          std::cout << pool.value(1000000000);
 *      } catch (const NotEqualTypeException& e) {
 *          std::cerr << e.message << std::endl;
 *          return 1;
 *      } catch (const NotStackHeadException& e) {
 *          std::cerr << e.message << std::endl;
 *          return 1;
 *      } catch (const EmptyStackException& e) {
 *          std::cerr << e.message << std::endl;
 *          return 1;
 *      } catch (const RangeCheckingException& e) {
 *          std::cerr << e.message << std::endl;
 *          return 1;
 *      }
 *  }
 *
 */