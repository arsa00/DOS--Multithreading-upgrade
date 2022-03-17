#ifndef _list_h_
#define _list_h_

template<class T>
class List
{
public:
	struct Element
	{
		T* ptr_to_elem;
		Element* next;
		Element* prev;

		Element() : ptr_to_elem(0), next(0), prev(0) { }
	};

	class Iterator
	{
	public:
		Iterator(List<T>::Element* el) : list(el) { }

		int hasNext()
		{
			return list != 0;
		}

	/*	int hasPrev()
		{
			return list != 0 && list->prev != 0;
		}*/

		T* next()
		{
			if (hasNext() == 0)
				return 0;
			T* ret = list->ptr_to_elem;
			list = list->next;

			return ret;
		}

		/*int prev()
		{
			if (hasPrev() == 0)
				return -1;
			list = list->prev;
			return 0;
		}*/
		// ovo bi mozda bilo dobro izbaciti
		T* getElement()
		{
			return list == 0 ? 0 : list->ptr_to_elem;
		}
	private:
		List<T>::Element* list;
	};

	List() : head(0), tail(0), size(0) { }

	int add(T* el)
	{
		Element* new_el = new Element();	// proveriti moze li
		new_el->ptr_to_elem = el;
		
		if (head == 0) head = tail = new_el;
		else
		{
			tail->next = new_el;
			new_el->prev = tail;
			tail = new_el;
		}
		size++;
		return 0;
	}

	int remove(T* el)
	{
		Element* ptr = head;
		while (ptr)
		{
			if (ptr->ptr_to_elem == el)
			{
				if (ptr->prev) ptr->prev->next = ptr->next;
				else head = ptr->next;

				if (ptr->next)
				{
					ptr->next->prev = ptr->prev;
				}
				else
				{
					tail = ptr->prev;
				}

				delete ptr;
				size--;
				return 0;	// uspesno
			}
			ptr = ptr->next;
		}
		
		return -1;	// neuspesno
	}

	Element* getIter()
	{
		if (head == 0) return 0;
		return head;
	}

	unsigned getSize() { return size; }

private:
	Element* head, * tail;
	unsigned size;
};

#endif
