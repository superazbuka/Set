//review
#include <memory>
#include <iostream>
#include <list>

namespace
{
template<typename Element>
class Node
{
private: using List = std::list<Element>;
private: using NodePtr = std::unique_ptr<Node<Element>>;
public: typedef typename List::const_iterator iterator;
private:
	Node *parent;
public:
	virtual iterator find_place(const Element &key) const = 0;
	virtual const iterator get_max() const = 0;
	virtual std::pair<NodePtr, NodePtr> insert(iterator) = 0;
	virtual std::pair<NodePtr, NodePtr> add_left(NodePtr) = 0;
	virtual std::pair<NodePtr, NodePtr> add_right(NodePtr) = 0;
	virtual std::pair<NodePtr, bool> erase(iterator) = 0;
	virtual ~Node() {}
};

template<typename Element>
class Leaf: public Node<Element>
{
private: using List = std::list<Element>;
private: using NodePtr = std::unique_ptr<Node<Element>>;
public: typedef typename List::const_iterator iterator;
private:
	iterator key;
public:
	Leaf(iterator new_key)
		: key(new_key)
	{}

	iterator find_place(const Element &) const override
	{
		return key;
	}

	const iterator get_max() const override
	{
		return key;
	}

	std::pair<NodePtr, NodePtr> insert(iterator cur) override
	{
		if (*cur < *key)
			swap(key, cur);
		return {NodePtr(new Leaf<Element>(key)), NodePtr (new Leaf<Element>(cur))};
	}

	std::pair<NodePtr, NodePtr> add_left(NodePtr) override
	{
		return {nullptr, NodePtr(new Leaf<Element>(key))};
	}

	std::pair<NodePtr, NodePtr> add_right(NodePtr) override
	{
		return {NodePtr(new Leaf<Element>(key)), nullptr};
	}

	std::pair<NodePtr, bool> erase(iterator) override
	{
		return {nullptr, false};
	}

};

template<typename Element>
class Triple: public Node<Element>
{
private: using List = std::list<Element>;
private: using NodePtr = std::unique_ptr<Node<Element>>;
public: typedef typename List::const_iterator iterator;
private:
	NodePtr first_child, second_child, third_child;
	iterator first_key, second_key, third_key;
public:
	Triple(NodePtr new_first_child, NodePtr new_second_child, NodePtr new_third_child)
		: first_child(std::move(new_first_child))
		, second_child(std::move(new_second_child))
		, third_child(std::move(new_third_child))
		, first_key(first_child->get_max())
		, second_key(second_child->get_max())
		, third_key(third_child->get_max())
	{}

	iterator find_place(const Element &cur_key) const override
	{
		if (*second_key < cur_key)
			return third_child->find_place(cur_key);
		else if (*first_key < cur_key)
			return second_child->find_place(cur_key);
		else
			return first_child->find_place(cur_key);
	}

	const iterator get_max() const override
	{
		return third_key;
	}

	std::pair<NodePtr, NodePtr> insert(iterator cur) override;

	std::pair<NodePtr, NodePtr> add_left(NodePtr cur) override;

	std::pair<NodePtr, NodePtr> add_right(NodePtr cur) override;
	
	std::pair<NodePtr, bool> erase(iterator cur) override;
};

template<typename Element>
class Double: public Node<Element>
{
private: using List = std::list<Element>;
private: using NodePtr = std::unique_ptr<Node<Element>>;
public: typedef typename List::const_iterator iterator;
private:
	NodePtr first_child, second_child;
	iterator first_key, second_key;
public:
	Double(NodePtr new_first_child, NodePtr new_second_child)
		: first_child(std::move(new_first_child))
		, second_child(std::move(new_second_child))
		, first_key(first_child->get_max())
		, second_key(second_child->get_max())
	{}
	iterator find_place(const Element &cur_key) const override
	{
		if (*first_key < cur_key)
			return second_child->find_place(cur_key);
		else
			return first_child->find_place(cur_key);
	}

	const iterator get_max() const override
	{
		return second_key;
	}

	std::pair<NodePtr, NodePtr> insert(iterator cur) override
	{
		if (*cur < *first_key)
		{
			auto x = first_child->insert(cur);
			if (x.second == nullptr)
			{
				return {NodePtr(new Double<Element>(std::move(x.first), std::move(second_child))), nullptr};
			}
			else
			{
				return {NodePtr(new Triple<Element>(std::move(x.first), std::move(x.second), std::move(second_child))), nullptr};
			}
		}
		else
		{
			auto x = second_child->insert(cur);
			if (x.second == nullptr)
			{
				return {NodePtr(new Double<Element>(std::move(first_child), std::move(x.first))), nullptr};
			}
			else
			{
				return {NodePtr(new Triple<Element>(std::move(first_child), std::move(x.first), std::move(x.second))), nullptr};
			}
		}
	}

	std::pair<NodePtr, NodePtr> add_left(NodePtr cur) override
	{
		return {nullptr, 
			NodePtr (new Triple<Element>(std::move(cur), std::move(first_child), std::move(second_child)))};
	}

	std::pair<NodePtr, NodePtr> add_right(NodePtr cur) override
	{
		return {NodePtr (new Triple<Element>(std::move(first_child), std::move(second_child), std::move(cur)))
			, nullptr};
	}

	std::pair<NodePtr, bool> erase(iterator cur) override
	{
		if (not (*first_key < *cur))
		{
			auto x = first_child->erase(cur);
			if (x.second)
			{
				return {NodePtr(new Double<Element>(std::move(x.first), std::move(second_child))), true};
			}
			else
			{
				auto y = second_child->add_left(std::move(x.first));
				if (y.first == nullptr)
				{
					return {std::move(y.second), false};
				}
				else
				{
					return {NodePtr (new Double<Element>(std::move(y.first), std::move(y.second))), true};
				}
			}
		}
		else
		{
			auto x = second_child->erase(cur);
			if (x.second)
			{
				return {NodePtr(new Double<Element>(std::move(first_child), std::move(x.first))), true};
			}
			else
			{
				auto y = first_child->add_right(std::move(x.first));
				if (y.second == nullptr)
				{
					return {std::move(y.first), false};
				}
				else
				{
					return {NodePtr (new Double<Element>(std::move(y.first), std::move(y.second))), true};
				}
			}
		}
	}
};

template<typename Element>
std::pair<std::unique_ptr<Node<Element>>, std::unique_ptr<Node<Element>>> Triple<Element>::insert(iterator cur)
{
	if (not (*first_key < *cur))
	{
		auto x = first_child->insert(cur);
		if (x.second == nullptr)
		{
			return {NodePtr (new Triple<Element>(std::move(x.first), std::move(second_child), std::move(third_child))), nullptr};
		}
		else
		{
			return {NodePtr (new Double<Element>(std::move(x.first), std::move(x.second))),
					NodePtr (new Double<Element>(std::move(second_child), std::move(third_child)))};
		}
	}
	else if (not (*second_key < *cur))
	{
		auto x = second_child->insert(cur);
		if (x.second == nullptr)
		{
			return {NodePtr (new Triple<Element>(std::move(first_child), std::move(x.first), std::move(third_child))), nullptr};
		}
		else
		{
			return {NodePtr (new Double<Element>(std::move(first_child), std::move(x.first))),
					NodePtr (new Double<Element>(std::move(x.second), std::move(third_child)))};
		}
	}
	else
	{
		auto x = third_child->insert(cur);
		if (x.second == nullptr)
		{
			return {NodePtr (new Triple<Element>(std::move(first_child), std::move(second_child), std::move(x.first))), nullptr};
		}
		else
		{
			return {NodePtr (new Double<Element>(std::move(first_child), std::move(second_child))),
					NodePtr (new Double<Element>(std::move(x.first), std::move(x.second)))};
		}
	}
}

template<typename Element>
std::pair<std::unique_ptr<Node<Element>>, std::unique_ptr<Node<Element>>> Triple<Element>::add_left(std::unique_ptr<Node<Element>> cur)
{
	return {NodePtr (new Double<Element>(std::move(cur), std::move(first_child))),
			NodePtr (new Double<Element>(std::move(second_child), std::move(third_child)))};
}

template<typename Element>
std::pair<std::unique_ptr<Node<Element>>, std::unique_ptr<Node<Element>>> Triple<Element>::add_right(std::unique_ptr<Node<Element>> cur)
{
	return {NodePtr (new Double<Element>(std::move(first_child), std::move(second_child))),
			NodePtr (new Double<Element>(std::move(third_child), std::move(cur)))};
}

template<typename Element>
std::pair<std::unique_ptr<Node<Element>>, bool> Triple<Element>::erase(iterator cur)
{
	if (not (*first_key < *cur))
	{
		auto x = first_child->erase(cur);
		if (x.second)
		{
			return {NodePtr (new Triple<Element>(std::move(x.first), std::move(second_child), std::move(third_child))), true};
		}
		else
		{
			auto y = second_child->add_left(std::move(x.first));
			if (y.first == nullptr)
			{
				return {NodePtr (new Double<Element>(std::move(y.second), std::move(third_child))), true};
			}
			else
			{
				return {NodePtr (new Triple<Element>(std::move(y.first), std::move(y.second), std::move(third_child))), true};
			}
		}
	}
	else if (not (*second_key < *cur))
	{
		auto x = second_child->erase(cur);
		if (x.second)
		{
			return {NodePtr (new Triple<Element>(std::move(first_child), std::move(x.first), std::move(third_child))), true};
		}
		else
		{
			auto y = third_child->add_left(std::move(x.first));
			if (y.first == nullptr)
			{
				return {NodePtr (new Double<Element>(std::move(first_child), std::move(y.second))), true};
			}
			else
			{
				return {NodePtr (new Triple<Element>(std::move(first_child), std::move(y.first), std::move(y.second))), true};
			}
		}
	}
	else
	{
		auto x = third_child->erase(cur);
		if (x.second)
		{
			return {NodePtr (new Triple<Element>(std::move(first_child), std::move(second_child), std::move(x.first))), true};
		}
		else
		{
			auto y = second_child->add_right(std::move(x.first));
			if (y.second == nullptr)
			{
				return {NodePtr (new Double<Element>(std::move(first_child), std::move(y.first))), true};
			}
			else
			{
				return {NodePtr (new Triple<Element>(std::move(first_child), std::move(y.first), std::move(y.second))), true};
			}
		}
	}
}
}

template<typename Element>
class Set
{
private: using List = std::list<Element>;
private: using NodePtr = std::unique_ptr<Node<Element>>;
public: typedef typename List::const_iterator iterator;
private:
	List elements;
	NodePtr root;
	size_t my_size;
public:
	Set()
		: elements()
		, root(nullptr)
		, my_size(0)
	{}

	Set(const Set<Element> &other)
		:Set(other.begin(), other.end())
	{}

	Set<Element> &operator =(const Set<Element> &other)
	{
		if (&other == this)
			return *this;
		else
		{
			elements.clear();
			root = nullptr;
			my_size = 0;
			for (auto i: other)
			{
				insert(i);
			}
			return *this;
		}
	}

	template<typename Iter>
	Set(Iter first, Iter last)
		: elements()
		, root(nullptr)
		, my_size(0)
	{
		for (; first != last; first++)
			insert(*first);
	}

	Set(std::initializer_list<Element> list)
		: elements()
		, root(nullptr)
		, my_size(0)
	{
		for (auto i: list)
			insert(i);
	}

	iterator find(const Element &key) const
	{
		if (root == nullptr)
			return elements.end();
		iterator cur = root->find_place(key);
		if (not (*cur < key) and not (key < *cur))
			return cur;
		else
			return elements.end();
	}

	iterator lower_bound(const Element &key) const
	{
		if (root == nullptr)
			return elements.end();
		iterator cur = root->find_place(key);
		if (not (*cur < key))
			return cur;
		else
			return elements.end();
	}

	void insert(const Element &key)
	{
		if (root != nullptr)
		{
			iterator it = root->find_place(key);
			if (it != elements.end() and not (*it < key) and not (key < *it))
				return;
			if (it != elements.end() and *it < key)
			{
				it++;
			}
			auto nit = elements.insert(it, key);
			++my_size;
			auto x = root->insert(nit);
			if (x.second == nullptr)
			{
				root = std::move(x.first);
			}
			else
			{
				root = NodePtr(new Double<Element>(std::move(x.first), std::move(x.second)));
			}
		}
		else
		{
			elements.push_back(key);
			++my_size;
			root = NodePtr(new Leaf<Element>(elements.begin()));
		}
	}

	void erase(const Element &key)
	{
		if (root != nullptr)
		{
			iterator it = find(key);
			if (it == elements.end())
				return;
			root = std::move(root->erase(it).first);
			elements.erase(it);
			my_size--;
		}
	}

	iterator begin() const
	{
		return elements.begin();
	}

	iterator end() const
	{
		return elements.end();
	}

	size_t size() const
	{
		return my_size;
	}

	bool empty() const
	{
		return my_size == 0;
	}
};
