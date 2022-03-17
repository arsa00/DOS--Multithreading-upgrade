#ifndef _list_semapore_h_
#define _list_semapore_h_

class KernelSem;

class ListSem
{
public:
	struct Element
	{
		KernelSem* ptr_to_elem;
		Element* next;
		Element* prev;

		Element() : ptr_to_elem(0), next(0), prev(0) { }
	};

	class Iterator
	{
	public:
		Iterator(ListSem::Element* el) : list(el) { }

		int hasNext()
		{
			return list != 0;
		}

		KernelSem* next()
		{
			if (hasNext() == 0)
				return 0;
			KernelSem* ret = list->ptr_to_elem;
			list = list->next;

			return ret;
		}

		// ovo bi mozda bilo dobro izbaciti
		KernelSem* getElement()
		{
			return list == 0 ? 0 : list->ptr_to_elem;
		}
	private:
		ListSem::Element* list;
	};

	ListSem() : head(0), tail(0), size(0) { }

	int add(KernelSem* el)
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

	int remove(KernelSem* el)
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
