#pragma once
template<typename T>
class enable_movable_ptr;

template<typename T>
class movable_ptr {
public:
	friend class enable_movable_ptr<T>;
	movable_ptr() :element(nullptr), next(nullptr), previous(nullptr) {}
	movable_ptr(enable_movable_ptr<T>* x) :element(x), next(nullptr), previous(nullptr)
	{
		if (element != nullptr)
		{
			element->addPointer(this);
		}
	}
	~movable_ptr() { reset(); }
	movable_ptr(const movable_ptr<T>& other) : movable_ptr(static_cast<T*>(other.element)) {}
	movable_ptr(movable_ptr<T>&& other) noexcept : movable_ptr(static_cast<T*>(other.element))
	{
		if (element != nullptr)
		{
			element->deletePointer(&other);
		}
	}
	movable_ptr<T>& operator=(const movable_ptr<T>& other);
	movable_ptr<T>& operator=(movable_ptr<T>&& other) noexcept;
	T* get() const;
	T& operator*() const;
	T* operator->() const;
	void reset();
	void reset(T* other);
	bool operator==(const movable_ptr<T>& x) const;
	bool operator!=(const movable_ptr<T>& x) const;
	operator bool() const;
	bool operator!() const;
private:
	void erase();
	bool oneItem() { return this->next == this; }
	movable_ptr<T>* addPtr(movable_ptr<T>* firstPointer);
	enable_movable_ptr<T>* element;
	movable_ptr<T>* next;
	movable_ptr<T>* previous;
};
//checks if left and right operands are the same if not, resets left one (so the orginal object is notified)
//and then changes where left one points, notifying object on which he points now.  
template<typename T>
movable_ptr<T>& movable_ptr<T>::operator=(const movable_ptr<T>& other)
{
	if (&other != this)
	{
		if (element != nullptr)
		{
			reset();
		}
		element = other.element;
		if (element != nullptr)
		{
			element->addPointer(this);
		}
	}
	return *this;
}
//checks if left and right operand are diferent, makes temporary movable_ptr<T> via move constructor
//and copies it to left operand.
template<typename T>
movable_ptr<T>& movable_ptr<T>::operator=(movable_ptr<T>&& other) noexcept
{
	if (&other != this)
	{
		movable_ptr<T> tmp(std::move(other));
		*this = tmp;
	}
	return *this;
}
template<typename T>
void movable_ptr<T>::reset()
{
	if (element != nullptr)
	{
		element->deletePointer(this);
	}
}
template<typename T>
void movable_ptr<T>::reset(T* other)
{
	reset();
	movable_ptr<T> tmp(other);
	*this = tmp;
}
template<typename T>
T* movable_ptr<T>::get() const
{
	return static_cast<T*>(element);
}
template<typename T>
T& movable_ptr<T>::operator*() const
{
	return *get();
}
template<typename T>
T* movable_ptr<T>::operator->() const
{
	return get();
}
template<typename T>
bool movable_ptr<T>::operator==(const movable_ptr<T>& x) const
{
	return this->element == x.element;
}
template<typename T>
bool movable_ptr<T>::operator!=(const movable_ptr<T>& x) const
{
	return !(*this == x);
}
template<typename T>
movable_ptr<T>::operator bool() const
{
	return element != nullptr;
}
template<typename T>
bool movable_ptr<T>::operator!() const
{
	return !((bool)*this);
}
template<typename T>
void movable_ptr<T>::erase()
{
	next->previous = this->previous;
	previous->next = this->next;
	element = nullptr;
	next = nullptr;
	previous = nullptr;
}
//returns movable_ptr<T> to the object which was passed as a parameter.
template<typename T>
movable_ptr<T> get_movable(T& x)
{
	movable_ptr<T> tmp(&x);
	return tmp;
}

template<typename T>
class enable_movable_ptr
{
public:
	friend class movable_ptr<T>;
	enable_movable_ptr() : first(nullptr) {}
	~enable_movable_ptr() { nullAllPointers(); }
	enable_movable_ptr(const enable_movable_ptr<T>& other) : first(nullptr) {};
	enable_movable_ptr(enable_movable_ptr<T>&& other) noexcept :first(nullptr)
	{
		other.redirectPointers(this);
	}
	enable_movable_ptr<T>& operator=(const enable_movable_ptr<T>& other);
	enable_movable_ptr<T>& operator=(enable_movable_ptr<T>&& other) noexcept;
private:
	void addPointer(movable_ptr<T>* x);
	void deletePointer(movable_ptr<T>* x);
	void redirectPointers(enable_movable_ptr<T>* whereTo);
	movable_ptr<T>* first;
	void nullAllPointers();
};
template<typename T>
enable_movable_ptr<T>& enable_movable_ptr<T>::operator=(const enable_movable_ptr<T>& other)
{
	if (this != &other)
	{
		nullAllPointers();
	}
	return *this;
}
template<typename T>
enable_movable_ptr<T>& enable_movable_ptr<T>::operator=(enable_movable_ptr<T>&& other) noexcept
{
	if (this != &other)
	{
		nullAllPointers();
		other.redirectPointers(this);
	}
	return *this;
}
template<typename T>
void enable_movable_ptr<T>::addPointer(movable_ptr<T>* x)
{
	first = x->addPtr(first);
}
//if the deleted pointer was the one the object remembered, we have to tell the new first p to the object
template<typename T>
void enable_movable_ptr<T>::deletePointer(movable_ptr<T>* x)
{
	if (x == first)
	{
		if (x->oneItem())
		{
			first = nullptr;
		}
		else
		{
			first = first->next;
		}
	}
	x->erase();
}
//removes first as long as there is some, changes where they point, tell the object they are pointing at it
template<typename T>
void enable_movable_ptr<T>::redirectPointers(enable_movable_ptr<T>* whereTo)
{
	while (first != nullptr)
	{
		movable_ptr<T>* tmp = first;
		first->reset();
		tmp->element = whereTo;
		if (whereTo != nullptr)
		{
			whereTo->addPointer(tmp);
		}
	}
}
template<typename T>
void enable_movable_ptr<T>::nullAllPointers()
{
	while (first != nullptr)
	{
		first->reset();
	}
}
//newP has to have filled the element pointer
template<typename T>
movable_ptr<T>* movable_ptr<T>::addPtr(movable_ptr<T>* firstPointer)
{
	if (firstPointer == nullptr)
	{
		firstPointer = this;
		this->next = this;
		this->previous = this;
	}
	else
	{
		this->previous = firstPointer->previous;
		this->next = firstPointer;
		this->previous->next = this;
		this->next->previous = this;
	}
	return this;
}